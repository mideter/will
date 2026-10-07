#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include "infra/transport/messenger.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>


namespace {


const char* g_server_exe = nullptr;


int connect_tcp(const char* host, std::uint16_t port)
{
	const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		return -1;

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if (::inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
		::close(fd);
		return -1;
	}

	if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
		::close(fd);
		return -1;
	}

	return fd;
}


using SessionStream = grpc::ClientReaderWriter<will::v1::ClientEvent, will::v1::ServerEvent>;


struct GrpcSession {
	std::shared_ptr<grpc::Channel> channel;
	std::unique_ptr<will::v1::Messenger::Stub> stub;
	std::unique_ptr<grpc::ClientContext> context = std::make_unique<grpc::ClientContext>();
	std::unique_ptr<SessionStream> stream;
};


GrpcSession open_session(const std::uint16_t port)
{
	GrpcSession session;
	const std::string target = "127.0.0.1:" + std::to_string(port);
	session.channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
	session.stub = will::v1::Messenger::NewStub(session.channel);
	// A test that waits for a word the server never sends fails instead of hanging.
	session.context->set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(20));
	session.stream = session.stub->Session(session.context.get());
	return session;
}


void drain_receipt_ack(SessionStream& stream)
{
	will::v1::ServerEvent event;
	REQUIRE(stream.Read(&event));
	REQUIRE(event.has_receipt_ack());
}


void send_chat(SessionStream& stream, const char* body)
{
	will::v1::ClientEvent event;
	event.mutable_chat()->set_body(body);
	REQUIRE(stream.Write(event));
}


void send_history_request(SessionStream& stream, std::uint32_t limit)
{
	will::v1::ClientEvent event;
	event.mutable_history_request()->set_limit(limit);
	REQUIRE(stream.Write(event));
}


constexpr const char* SenderToken = "11111111111111111111111111111111";
constexpr const char* ViewerToken = "22222222222222222222222222222222";


std::uint16_t pick_port()
{
	return static_cast<std::uint16_t>(20000 + (getpid() % 10000));
}


/// An empty directory for the databases of one server.
void fresh_directory(const std::string& directory)
{
	std::filesystem::remove_all(directory);
	std::filesystem::create_directories(directory);
}


pid_t start_server(const char* server_exe, std::uint16_t port, const std::string& db_path)
{
	const pid_t pid = fork();
	if (pid != 0)
		return pid;

	const std::string port_str = std::to_string(port);
	execl(server_exe, server_exe, "--port", port_str.c_str(), "--db-path", db_path.c_str(),
		  static_cast<char*>(nullptr));
	_exit(127);
}


void stop_server(pid_t pid)
{
	kill(pid, SIGTERM);
	int status = 0;
	waitpid(pid, &status, 0);
}


/// A server started for one test case; stopped when the case ends, even when it fails.
class RunningServer {
public:
	explicit RunningServer(const pid_t pid) : pid_(pid) {}
	~RunningServer() { stop(); }

	RunningServer(const RunningServer&) = delete;
	RunningServer& operator=(const RunningServer&) = delete;

	pid_t pid() const noexcept { return pid_; }

	void stop()
	{
		if (pid_ > 0)
			stop_server(pid_);
		pid_ = 0;
	}

	/// The case has stopped it itself.
	void forget() noexcept { pid_ = 0; }

private:
	pid_t pid_;
};


void wait_for_server(std::uint16_t port)
{
	for (int attempt = 0; attempt < 50; ++attempt) {
		const int fd = connect_tcp("127.0.0.1", port);
		if (fd >= 0) {
			::close(fd);
			return;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	FAIL("server did not start");
}


std::string bind_named(SessionStream& stream, const char* device_token)
{
	will::v1::ClientEvent event;
	event.mutable_bind_token()->set_token(device_token);
	REQUIRE(stream.Write(event));

	will::v1::ServerEvent response;
	REQUIRE(stream.Read(&response));
	REQUIRE(response.has_auth_ok());
	return response.auth_ok().name();
}


will::v1::ServerEvent read_event(SessionStream& stream)
{
	will::v1::ServerEvent event;
	REQUIRE(stream.Read(&event));
	return event;
}


void send_turn_to_abode(SessionStream& stream, const std::string& host, const std::string& room = {})
{
	will::v1::ClientEvent event;
	event.mutable_turn()->set_abode_of(host);
	event.mutable_turn()->set_room(room);
	REQUIRE(stream.Write(event));
}


/// Turn into a room of an abode (one's own when host is empty), and read what it
/// shows up to its end.
void stand_in(SessionStream& stream, const std::string& host, const std::string& room)
{
	send_turn_to_abode(stream, host, room);
	will::v1::ServerEvent event;
	do {
		REQUIRE(stream.Read(&event));
	} while (event.has_threshold());  // words about gates one stood in before
	REQUIRE(event.has_turned());
	do {
		REQUIRE(stream.Read(&event));
	} while (!event.has_history_end());
}


/// Turn into one's own cell, where one writes.
void enter_cell(SessionStream& stream)
{
	stand_in(stream, {}, "Келья");
}


/// The next event that is not a word about the gates.
will::v1::ServerEvent next_past_gates(SessionStream& stream)
{
	will::v1::ServerEvent event;
	do {
		REQUIRE(stream.Read(&event));
	} while (event.has_threshold());
	return event;
}


void send_admit(SessionStream& stream, const std::string& name)
{
	will::v1::ClientEvent admit;
	admit.mutable_admit()->set_name(name);
	REQUIRE(stream.Write(admit));
}


void send_regard(SessionStream& stream, const std::string& name, const will::v1::DwellerKind kind)
{
	will::v1::ClientEvent regard;
	regard.mutable_regard()->set_name(name);
	regard.mutable_regard()->set_kind(kind);
	REQUIRE(stream.Write(regard));
}


constexpr const char* ElderToken = "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee";


/// The first man of the server: begotten at once, he bears the others in his
/// birth room, that no man of a test is the father of another.
struct Elder {
	GrpcSession session;
	std::string name;
};


Elder first_man(const std::uint16_t port)
{
	Elder elder{open_session(port), {}};
	REQUIRE(elder.session.stream);
	elder.name = bind_named(*elder.session.stream, ElderToken);
	REQUIRE_FALSE(elder.name.empty());
	return elder;
}


void send_bear(SessionStream& stream, const std::uint64_t mark)
{
	will::v1::ClientEvent bear;
	bear.mutable_bear()->set_mark(mark);
	REQUIRE(stream.Write(bear));
}


/// Bind a new body and have the elder bear it; the name of the man born.
std::string born_named(Elder& elder, SessionStream& stream, const char* device_token)
{
	will::v1::ClientEvent event;
	event.mutable_bind_token()->set_token(device_token);
	REQUIRE(stream.Write(event));
	const will::v1::ServerEvent welcomed = read_event(stream);
	REQUIRE(welcomed.has_auth_ok());
	REQUIRE(welcomed.auth_ok().unborn());

	SessionStream& midwife = *elder.session.stream;
	stand_in(midwife, {}, "Родильная");
	send_bear(midwife, welcomed.auth_ok().mark());
	CHECK(read_event(midwife).has_protocol_notice());
	CHECK(read_event(midwife).has_dwelling());  // he dwells in the child's abode as a neighbour
	CHECK(read_event(midwife).has_unborn());
	stand_in(midwife, {}, {});

	const will::v1::ServerEvent born = read_event(stream);
	REQUIRE(born.has_auth_ok());
	CHECK_FALSE(born.auth_ok().unborn());
	CHECK(read_event(stream).dwelling().host_name() == elder.name);
	return born.auth_ok().name();
}


} // namespace


TEST_CASE("history request returns letters of the witness abode with is_mine")
{
	const std::uint16_t port = pick_port();
	const std::string db_path = "/tmp/will-history-test-" + std::to_string(getpid());

	fresh_directory(db_path);

	RunningServer server{start_server(g_server_exe, port, db_path)};
	const pid_t server_pid = server.pid();
	REQUIRE(server_pid > 0);
	wait_for_server(port);

	Elder elder = first_man(port);
	GrpcSession sender = open_session(port);
	REQUIRE(sender.stream);
	born_named(elder, *sender.stream, SenderToken);
	enter_cell(*sender.stream);
	send_chat(*sender.stream, "hello-from-sender");
	drain_receipt_ack(*sender.stream);

	GrpcSession viewer = open_session(port);
	REQUIRE(viewer.stream);
	born_named(elder, *viewer.stream, ViewerToken);
	send_history_request(*viewer.stream, 10);

	// Viewer looks at his own abode itself: its rooms and what awaits him, no words.
	will::v1::ServerEvent viewer_rooms;
	REQUIRE(viewer.stream->Read(&viewer_rooms));
	REQUIRE(viewer_rooms.has_rooms());
	REQUIRE(viewer_rooms.rooms().rooms_size() == 4);
	CHECK(viewer_rooms.rooms().rooms(0).name() == "Келья");
	CHECK(viewer_rooms.rooms().rooms(0).part() == will::v1::INNER);
	CHECK(viewer_rooms.rooms().rooms(1).name() == "Врата");
	CHECK(viewer_rooms.rooms().rooms(2).name() == "Горница");
	CHECK(viewer_rooms.rooms().rooms(3).name() == "Родильная");
	will::v1::ServerEvent viewer_outstanding;
	REQUIRE(viewer.stream->Read(&viewer_outstanding));
	CHECK(viewer_outstanding.has_outstanding());
	will::v1::ServerEvent viewer_end;
	REQUIRE(viewer.stream->Read(&viewer_end));
	CHECK(viewer_end.has_history_end());

	send_chat(*sender.stream, "hello-again");
	drain_receipt_ack(*sender.stream);
	send_history_request(*sender.stream, 10);

	will::v1::ServerEvent own_item;
	REQUIRE(sender.stream->Read(&own_item));
	REQUIRE(own_item.has_word());
	CHECK(own_item.word().body() == "hello-from-sender");
	CHECK(own_item.word().is_mine());
	CHECK_FALSE(own_item.word().name().empty());
	CHECK(own_item.word().name().size() == 8);

	will::v1::ServerEvent own_second;
	REQUIRE(sender.stream->Read(&own_second));
	REQUIRE(own_second.has_word());
	CHECK(own_second.word().body() == "hello-again");
	CHECK(own_second.word().is_mine());
	CHECK(own_second.word().name() == own_item.word().name());

	will::v1::ServerEvent sender_end;
	REQUIRE(sender.stream->Read(&sender_end));
	CHECK(sender_end.has_history_end());

	sender.context->TryCancel();
	viewer.context->TryCancel();
	elder.session.context->TryCancel();
	server.stop();
	std::filesystem::remove_all(db_path);
}


TEST_CASE("a host admits a dweller, regards him anew, and he sees his abode as his kind does")
{
	const std::uint16_t port = static_cast<std::uint16_t>(pick_port() + 1);
	const std::string directory = "/tmp/will-dwellers-test-" + std::to_string(getpid());
	fresh_directory(directory);

	RunningServer server{start_server(g_server_exe, port, directory)};
	const pid_t server_pid = server.pid();
	REQUIRE(server_pid > 0);
	wait_for_server(port);

	Elder elder = first_man(port);
	GrpcSession host = open_session(port);
	REQUIRE(host.stream);
	const std::string host_name = born_named(elder, *host.stream, SenderToken);
	enter_cell(*host.stream);
	send_chat(*host.stream, "secret");
	drain_receipt_ack(*host.stream);

	GrpcSession man = open_session(port);
	REQUIRE(man.stream);
	const std::string man_name = born_named(elder, *man.stream, ViewerToken);

	// A stranger cannot turn to the abode.
	send_turn_to_abode(*man.stream, host_name);
	CHECK(read_event(*man.stream).has_protocol_notice());

	// He comes to the gates: they are shut, the host is not there and cannot let him in.
	send_turn_to_abode(*man.stream, host_name, "Врата");
	const will::v1::ServerEvent at_gates = read_event(*man.stream);
	CHECK(at_gates.turned().aspect() == will::v1::THRESHOLD);
	const will::v1::ServerEvent shut = read_event(*man.stream);
	REQUIRE(shut.has_threshold());
	CHECK_FALSE(shut.threshold().open());
	CHECK(read_event(*man.stream).has_history_end());
	send_admit(*host.stream, man_name);
	CHECK(read_event(*host.stream).has_protocol_notice());

	// The host comes into his gates: he sees who waits, and the one waiting sees them open.
	send_turn_to_abode(*host.stream, {}, "Врата");
	CHECK(read_event(*host.stream).has_turned());
	const will::v1::ServerEvent waiting = read_event(*host.stream);
	REQUIRE(waiting.threshold().waiting_size() == 1);
	CHECK(waiting.threshold().waiting(0) == man_name);
	CHECK(read_event(*host.stream).has_history_end());
	CHECK(read_event(*man.stream).threshold().open());

	// He lets him in: the one let in is told he dwells there as an acquaintance; no one waits now.
	send_admit(*host.stream, man_name);
	CHECK(read_event(*host.stream).has_protocol_notice());
	CHECK(read_event(*host.stream).threshold().waiting_size() == 0);
	const will::v1::ServerEvent dwelling = read_event(*man.stream);
	REQUIRE(dwelling.has_dwelling());
	CHECK(dwelling.dwelling().host_name() == host_name);
	CHECK(dwelling.dwelling().kind() == will::v1::ACQUAINTANCE);
	CHECK(read_event(*man.stream).has_threshold());

	send_turn_to_abode(*man.stream, host_name);
	CHECK(read_event(*host.stream).has_threshold());  // he left the gates the host stands in
	const will::v1::ServerEvent turned = read_event(*man.stream);
	REQUIRE(turned.has_turned());
	CHECK(turned.turned().abode_of() == host_name);
	const will::v1::ServerEvent shown = read_event(*man.stream);
	REQUIRE(shown.rooms().rooms_size() == 1);  // only the gates, open to anyone
	CHECK(shown.rooms().rooms(0).name() == "Врата");
	CHECK(read_event(*man.stream).has_history_end());

	// He may look, but not write there, nor enter the inner cell.
	send_chat(*man.stream, "not mine");
	CHECK(read_event(*man.stream).has_protocol_notice());
	send_turn_to_abode(*man.stream, host_name, "Келья");
	CHECK(read_event(*man.stream).has_protocol_notice());

	// Outside his upper room the host may not regard him anew; in it he may. A friend is
	// shown every room at once: the cell, the gates, the upper room.
	send_regard(*host.stream, man_name, will::v1::FRIEND);
	CHECK(read_event(*host.stream).has_protocol_notice());
	stand_in(*host.stream, {}, "Горница");
	send_regard(*host.stream, man_name, will::v1::FRIEND);
	CHECK(read_event(*host.stream).has_protocol_notice());
	const will::v1::ServerEvent upper_room = read_event(*host.stream);
	REQUIRE(upper_room.dwellers().dwellers_size() == 2);  // and the elder, his father by flesh
	for (const will::v1::Dweller& dweller : upper_room.dwellers().dwellers())
		CHECK(dweller.kind() == (dweller.name() == man_name ? will::v1::FRIEND : will::v1::NEIGHBOUR));
	CHECK(read_event(*man.stream).dwelling().kind() == will::v1::FRIEND);
	REQUIRE(read_event(*man.stream).turned().abode_of() == host_name);
	const will::v1::ServerEvent open = read_event(*man.stream);
	REQUIRE(open.rooms().rooms_size() == 4);
	CHECK(open.rooms().rooms(0).name() == "Келья");
	CHECK(read_event(*man.stream).has_history_end());

	// Entering the cell, he sees the words.
	send_turn_to_abode(*man.stream, host_name, "Келья");
	CHECK(read_event(*man.stream).turned().room() == "Келья");
	const will::v1::ServerEvent seen = read_event(*man.stream);
	REQUIRE(seen.has_word());
	CHECK(seen.word().body() == "secret");
	CHECK_FALSE(seen.word().is_mine());
	CHECK(read_event(*man.stream).has_history_end());

	// What the host says next in his cell reaches him at once.
	enter_cell(*host.stream);
	send_chat(*host.stream, "news");
	drain_receipt_ack(*host.stream);
	const will::v1::ServerEvent live = read_event(*man.stream);
	REQUIRE(live.has_word());
	CHECK(live.word().body() == "news");

	// Regarded as an acquaintance again, he sees the abode anew at once: no words.
	stand_in(*host.stream, {}, "Горница");
	send_regard(*host.stream, man_name, will::v1::ACQUAINTANCE);
	CHECK(read_event(*host.stream).has_protocol_notice());
	CHECK(read_event(*host.stream).has_dwellers());
	CHECK(read_event(*man.stream).dwelling().kind() == will::v1::ACQUAINTANCE);
	REQUIRE(read_event(*man.stream).turned().room() == "Келья");
	CHECK(read_event(*man.stream).has_history_end());

	// The host sets his cell in the outer part; a neighbour, looking at it, sees the words again.
	send_regard(*host.stream, man_name, will::v1::NEIGHBOUR);
	CHECK(read_event(*host.stream).has_protocol_notice());
	CHECK(read_event(*host.stream).has_dwellers());
	CHECK(read_event(*man.stream).dwelling().kind() == will::v1::NEIGHBOUR);
	REQUIRE(read_event(*man.stream).turned().room() == "Келья");
	CHECK(read_event(*man.stream).has_history_end());
	{
		will::v1::ClientEvent arrange;
		arrange.mutable_arrange()->set_room("Келья");
		arrange.mutable_arrange()->set_part(will::v1::OUTER);
		REQUIRE(host.stream->Write(arrange));
	}
	CHECK(read_event(*host.stream).has_protocol_notice());
	REQUIRE(read_event(*man.stream).turned().room() == "Келья");
	CHECK(read_event(*man.stream).word().body() == "secret");
	CHECK(read_event(*man.stream).word().body() == "news");
	CHECK(read_event(*man.stream).has_history_end());

	// The host lists his dwellers.
	{
		will::v1::ClientEvent list;
		list.mutable_list_dwellers();
		REQUIRE(host.stream->Write(list));
	}
	const will::v1::ServerEvent dwellers = read_event(*host.stream);
	REQUIRE(dwellers.has_dwellers());
	REQUIRE(dwellers.dwellers().dwellers_size() == 2);
	for (const will::v1::Dweller& dweller : dwellers.dwellers().dwellers())
		CHECK(dweller.kind() == will::v1::NEIGHBOUR);

	host.context->TryCancel();
	man.context->TryCancel();
	elder.session.context->TryCancel();
	server.stop();
	std::filesystem::remove_all(directory);
}


TEST_CASE("one lists the abodes one dwells in and the supplications awaiting one, and rejects one")
{
	const std::uint16_t port = static_cast<std::uint16_t>(pick_port() + 2);
	const std::string directory = "/tmp/will-lists-test-" + std::to_string(getpid());
	fresh_directory(directory);

	RunningServer server{start_server(g_server_exe, port, directory)};
	REQUIRE(server.pid() > 0);
	wait_for_server(port);

	Elder elder = first_man(port);
	GrpcSession a = open_session(port);
	REQUIRE(a.stream);
	const std::string a_name = born_named(elder, *a.stream, SenderToken);
	GrpcSession b = open_session(port);
	REQUIRE(b.stream);
	const std::string b_name = born_named(elder, *b.stream, ViewerToken);

	// They let each other in, each standing in his gates with the other at them.
	stand_in(*a.stream, {}, "Врата");
	stand_in(*b.stream, a_name, "Врата");
	send_admit(*a.stream, b_name);
	CHECK(next_past_gates(*a.stream).has_protocol_notice());
	CHECK(next_past_gates(*b.stream).has_dwelling());
	stand_in(*b.stream, {}, "Врата");
	stand_in(*a.stream, b_name, "Врата");
	send_admit(*b.stream, a_name);
	CHECK(next_past_gates(*b.stream).has_protocol_notice());
	CHECK(next_past_gates(*a.stream).has_dwelling());

	// b dwells with a, as an acquaintance.
	{
		will::v1::ClientEvent list;
		list.mutable_list_dwellings();
		REQUIRE(b.stream->Write(list));
	}
	const will::v1::ServerEvent dwellings = next_past_gates(*b.stream);
	REQUIRE(dwellings.has_dwellings());
	REQUIRE(dwellings.dwellings().dwellings_size() == 2);  // and with the elder, his father
	bool with_a = false;
	for (const will::v1::Dwelling& dwelling : dwellings.dwellings().dwellings()) {
		CHECK(dwelling.kind() == will::v1::ACQUAINTANCE);
		with_a = with_a || dwelling.host_name() == a_name;
	}
	CHECK(with_a);

	// b supplicates a; a sees it awaiting and rejects it.
	{
		will::v1::ClientEvent ask;
		ask.mutable_supplicate()->set_addressee_name(a_name);
		REQUIRE(b.stream->Write(ask));
	}
	CHECK(next_past_gates(*b.stream).has_protocol_notice());
	CHECK(next_past_gates(*a.stream).has_supplication_offer());
	{
		will::v1::ClientEvent list;
		list.mutable_list_supplications();
		REQUIRE(a.stream->Write(list));
	}
	const will::v1::ServerEvent awaiting = next_past_gates(*a.stream);
	REQUIRE(awaiting.has_supplications());
	REQUIRE(awaiting.supplications().suppliant_names_size() == 1);
	CHECK(awaiting.supplications().suppliant_names(0) == b_name);
	{
		will::v1::ClientEvent reject;
		reject.mutable_reject_supplication()->set_suppliant_name(b_name);
		REQUIRE(a.stream->Write(reject));
	}
	CHECK(next_past_gates(*a.stream).has_protocol_notice());
	{
		will::v1::ClientEvent list;
		list.mutable_list_supplications();
		REQUIRE(a.stream->Write(list));
	}
	CHECK(next_past_gates(*a.stream).supplications().suppliant_names_size() == 0);

	// b asks for rooms without moving his gaze: his own three; in a's abode, as an acquaintance, only the gates.
	{
		will::v1::ClientEvent list;
		list.mutable_list_rooms();
		REQUIRE(b.stream->Write(list));
	}
	const will::v1::ServerEvent own_rooms = next_past_gates(*b.stream);
	REQUIRE(own_rooms.has_rooms());
	REQUIRE(own_rooms.rooms().rooms_size() == 4);
	CHECK(own_rooms.rooms().rooms(0).name() == "Келья");
	{
		will::v1::ClientEvent list;
		list.mutable_list_rooms()->set_abode_of(a_name);
		REQUIRE(b.stream->Write(list));
	}
	const will::v1::ServerEvent their_rooms = next_past_gates(*b.stream);
	REQUIRE(their_rooms.rooms().rooms_size() == 1);
	CHECK(their_rooms.rooms().rooms(0).name() == "Врата");

	a.context->TryCancel();
	b.context->TryCancel();
	elder.session.context->TryCancel();
	server.stop();
	std::filesystem::remove_all(directory);
}


TEST_CASE("an unborn body waits, seen in birth rooms; it is born of the host of the room; a man chooses his father by spirit")
{
	const std::uint16_t port = static_cast<std::uint16_t>(pick_port() + 3);
	const std::string directory = "/tmp/will-birth-test-" + std::to_string(getpid());
	fresh_directory(directory);

	RunningServer server{start_server(g_server_exe, port, directory)};
	REQUIRE(server.pid() > 0);
	wait_for_server(port);

	// The first comes into the world at once.
	Elder elder = first_man(port);
	SessionStream& adam = *elder.session.stream;

	// Standing in his birth room, he sees no one awaiting.
	send_turn_to_abode(adam, {}, "Родильная");
	CHECK(read_event(adam).turned().aspect() == will::v1::BIRTH);
	CHECK(read_event(adam).unborn().marks_size() == 0);
	CHECK(read_event(adam).has_history_end());

	// A body comes after him: it awaits its birth and only waits.
	GrpcSession seth = open_session(port);
	REQUIRE(seth.stream);
	{
		will::v1::ClientEvent bind;
		bind.mutable_bind_token()->set_token(SenderToken);
		REQUIRE(seth.stream->Write(bind));
	}
	const will::v1::ServerEvent welcomed = read_event(*seth.stream);
	REQUIRE(welcomed.auth_ok().unborn());
	CHECK(welcomed.auth_ok().name().empty());
	const std::uint64_t mark = welcomed.auth_ok().mark();
	send_turn_to_abode(*seth.stream, {}, "Келья");
	CHECK(read_event(*seth.stream).protocol_notice().message() == "one awaits one's birth");

	// Adam sees him awaiting at once, and bears him.
	const will::v1::ServerEvent awaiting = read_event(adam);
	REQUIRE(awaiting.unborn().marks_size() == 1);
	CHECK(awaiting.unborn().marks(0) == mark);
	send_bear(adam, mark);
	const std::string born_notice = read_event(adam).protocol_notice().message();
	CHECK(read_event(adam).has_dwelling());
	CHECK(read_event(adam).unborn().marks_size() == 0);

	const will::v1::ServerEvent born = read_event(*seth.stream);
	REQUIRE(born.has_auth_ok());
	const std::string seth_name = born.auth_ok().name();
	CHECK(born_notice == seth_name + " born");
	const will::v1::ServerEvent dwelling = read_event(*seth.stream);
	CHECK(dwelling.dwelling().host_name() == elder.name);
	CHECK(dwelling.dwelling().kind() == will::v1::ACQUAINTANCE);

	// One man at a time stands in a birth room: Adam is in his, so Seth's own is open
	// to Seth, but Seth, a stranger to Adam's inner part, may not enter Adam's.
	stand_in(*seth.stream, {}, "Родильная");
	send_turn_to_abode(*seth.stream, elder.name, "Родильная");
	CHECK(read_event(*seth.stream).has_protocol_notice());

	// Seth chooses Adam as his father by spirit; Adam sees his line and is told.
	{
		will::v1::ClientEvent choose;
		choose.mutable_choose_father()->set_name(elder.name);
		REQUIRE(seth.stream->Write(choose));
	}
	CHECK(read_event(*seth.stream).has_protocol_notice());
	CHECK(read_event(adam).has_protocol_notice());
	{
		will::v1::ClientEvent list;
		list.mutable_list_lineage();
		REQUIRE(adam.Write(list));
	}
	const will::v1::ServerEvent line = read_event(adam);
	REQUIRE(line.lineage().descents_size() == 1);
	CHECK(line.lineage().descents(0).name() == seth_name);
	CHECK(line.lineage().descents(0).father_name() == elder.name);

	seth.context->TryCancel();
	elder.session.context->TryCancel();
	server.stop();
	std::filesystem::remove_all(directory);
}


int main(int argc, char** argv)
{
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <path-to-will-server>\n";
		return EXIT_FAILURE;
	}

	g_server_exe = argv[1];

	doctest::Context context;
	std::vector<char*> doctest_argv{argv[0]};

	for (int i = 2; i < argc; ++i)
		doctest_argv.push_back(argv[i]);

	context.applyCommandLine(static_cast<int>(doctest_argv.size()), doctest_argv.data());
	return context.run();
}
