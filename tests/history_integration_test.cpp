#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include "infra/transport/messenger.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <csignal>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <system_error>
#include <optional>
#include <thread>
#include <tuple>
#include <utility>
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
	// Each session its own connection, as each phone has: one left from a server gone is not shared.
	grpc::ChannelArguments own;
	own.SetInt(GRPC_ARG_USE_LOCAL_SUBCHANNEL_POOL, 1);
	session.channel = grpc::CreateCustomChannel(target, grpc::InsecureChannelCredentials(), own);
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


/// An empty directory for the databases of one test, gone when the test ends —
/// even when it fails.
class ScratchDirectory {
public:
	explicit ScratchDirectory(std::string path)
		: path_(std::move(path))
	{
		std::filesystem::remove_all(path_);
		std::filesystem::create_directories(path_);
	}

	~ScratchDirectory()
	{
		std::error_code error;
		std::filesystem::remove_all(path_, error);
	}

	ScratchDirectory(const ScratchDirectory&) = delete;
	ScratchDirectory& operator=(const ScratchDirectory&) = delete;

private:
	std::string path_;
};


pid_t start_server(const char* server_exe, std::uint16_t port, const std::string& db_path)
{
	const pid_t pid = fork();
	if (pid != 0)
		return pid;
	// The child dies with the test, even when the test dies first (a failed check
	// that aborts it): no server is left behind holding the port and the output.
	prctl(PR_SET_PDEATHSIG, SIGKILL);

	const std::string port_str = std::to_string(port);
	execl(server_exe, server_exe, "--port", port_str.c_str(), "--db-path", db_path.c_str(),
		  static_cast<char*>(nullptr));
	_exit(127);
}


constexpr const char* TestCert = R"pem(-----BEGIN CERTIFICATE-----
MIIBkDCCATagAwIBAgIULNzGmAlQ3iiu+Er6FQowBIGCr9owCgYIKoZIzj0EAwIw
FDESMBAGA1UEAwwJd2lsbC10ZXN0MCAXDTI2MTAxMzEwMzYzMFoYDzIxMjYwOTE5
MTAzNjMwWjAUMRIwEAYDVQQDDAl3aWxsLXRlc3QwWTATBgcqhkjOPQIBBggqhkjO
PQMBBwNCAAToVYKmitqtINK5tc7TwB7j4SQIoxqPs8jzKMLT3k0uWHQd1yFjVJ9B
y+e2KQJRzQkHdwyCHkX4n9o6Xk4g9HqRo2QwYjAdBgNVHQ4EFgQUC1XRW+6TAKQu
dsp9AbPwnQYmABIwHwYDVR0jBBgwFoAUC1XRW+6TAKQudsp9AbPwnQYmABIwDwYD
VR0TAQH/BAUwAwEB/zAPBgNVHREECDAGhwR/AAABMAoGCCqGSM49BAMCA0gAMEUC
IQDwjDCGVNLffEFWt2HuCkXX6F7MSz1bfjcWcfGYnaQH/QIgVg/bhXETbZQQ2Xvx
/RX3VN52q+kY/d8tZO8dcEaEazE=
-----END CERTIFICATE-----
)pem";
constexpr const char* TestKey = R"pem(-----BEGIN PRIVATE KEY-----
MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgf4a3DxyncLHrPJkK
IlHut7z/5HzdqDZJU1uoE9+xnJGhRANCAAToVYKmitqtINK5tc7TwB7j4SQIoxqP
s8jzKMLT3k0uWHQd1yFjVJ9By+e2KQJRzQkHdwyCHkX4n9o6Xk4g9HqR
-----END PRIVATE KEY-----
)pem";


void write_file(const std::string& path, const char* text)
{
	std::ofstream out{path};
	out << text;
}


pid_t start_server_with(const char* server_exe, const std::vector<std::string>& args)
{
	const pid_t pid = fork();
	if (pid != 0)
		return pid;
	// The child dies with the test, even when the test dies first (a failed check
	// that aborts it): no server is left behind holding the port and the output.
	prctl(PR_SET_PDEATHSIG, SIGKILL);

	std::vector<char*> argv{const_cast<char*>(server_exe)};
	for (const std::string& arg : args)
		argv.push_back(const_cast<char*>(arg.c_str()));
	argv.push_back(nullptr);
	execv(server_exe, argv.data());
	_exit(127);
}


GrpcSession open_tls_session(const std::uint16_t port, const char* cert)
{
	GrpcSession session;
	grpc::SslCredentialsOptions tls;
	tls.pem_root_certs = cert;
	session.channel = grpc::CreateChannel("127.0.0.1:" + std::to_string(port), grpc::SslCredentials(tls));
	session.stub = will::v1::Messenger::NewStub(session.channel);
	session.context->set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(20));
	session.stream = session.stub->Session(session.context.get());
	return session;
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
	CHECK(response.auth_ok().father().empty());  // the father is told only as one is born
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


void stand_in(SessionStream& stream, const std::string& host, const std::string& room)
{
	send_turn_to_abode(stream, host, room);
	will::v1::ServerEvent event;
	do {
		REQUIRE(stream.Read(&event));
	} while (event.has_threshold() || event.has_unborn());  // words about gates one stood in before
	REQUIRE(event.has_turned());
	do {
		REQUIRE(stream.Read(&event));
	} while (!event.has_history_end());
}


void send_knock(SessionStream& stream)
{
	will::v1::ClientEvent knock;
	knock.mutable_knock();
	REQUIRE(stream.Write(knock));
}


void knock_at_gates(SessionStream& stream, const std::string& host)
{
	stand_in(stream, {}, "Врата");
	stand_in(stream, host, "Врата");
	send_knock(stream);
	REQUIRE(read_event(stream).threshold().knocking());
}


void enter_cell(SessionStream& stream)
{
	stand_in(stream, {}, "Слово");
}


will::v1::ServerEvent next_past_gates(SessionStream& stream)
{
	will::v1::ServerEvent event;
	do {
		REQUIRE(stream.Read(&event));
	} while (event.has_threshold() || event.has_unborn());
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


/// The first man of the server: begotten at once, he bears the others at his
/// gates, that no man of a test is the father of another.
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


std::string born_named(Elder& elder, SessionStream& stream, const char* device_token)
{
	will::v1::ClientEvent event;
	event.mutable_bind_token()->set_token(device_token);
	REQUIRE(stream.Write(event));
	const will::v1::ServerEvent welcomed = read_event(stream);
	REQUIRE(welcomed.has_auth_ok());
	REQUIRE(welcomed.auth_ok().unborn());
	CHECK(welcomed.auth_ok().father().empty());

	SessionStream& midwife = *elder.session.stream;
	stand_in(midwife, {}, "Врата");
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


template<typename Question>
will::v1::ServerEvent read_until(SessionStream& stream, Question question)
{
	for (int read = 0; read < 50; ++read) {
		will::v1::ServerEvent event;
		REQUIRE(stream.Read(&event));
		if (question(event))
			return event;
	}
	FAIL("the awaited event did not come");
	return {};
}


} // namespace


TEST_CASE("history request returns letters of the witness abode with is_mine")
{
	const std::uint16_t port = pick_port();
	const std::string db_path = "/tmp/will-history-test-" + std::to_string(getpid());

	const ScratchDirectory scratch{db_path};

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
	{
		// The receipt carries the word itself: the author is not told it otherwise.
		will::v1::ServerEvent receipt;
		REQUIRE(sender.stream->Read(&receipt));
		REQUIRE(receipt.has_receipt_ack());
		REQUIRE(receipt.receipt_ack().has_word());
		CHECK(receipt.receipt_ack().word().body() == "hello-from-sender");
		CHECK(receipt.receipt_ack().word().is_mine());
		CHECK(receipt.receipt_ack().word().kind() == will::v1::Word::LETTER);
		CHECK(receipt.receipt_ack().word().id() != 0);
	}

	GrpcSession viewer = open_session(port);
	REQUIRE(viewer.stream);
	born_named(elder, *viewer.stream, ViewerToken);
	send_history_request(*viewer.stream, 10);

	// Viewer looks at his own abode itself: its rooms and what awaits him, no words.
	will::v1::ServerEvent viewer_rooms;
	REQUIRE(viewer.stream->Read(&viewer_rooms));
	REQUIRE(viewer_rooms.has_rooms());
	REQUIRE(viewer_rooms.rooms().rooms_size() == 3);
	CHECK(viewer_rooms.rooms().rooms(0).name() == "Слово");
	CHECK(viewer_rooms.rooms().rooms(0).part() == will::v1::INNER);
	CHECK(viewer_rooms.rooms().rooms(1).name() == "Врата");
	CHECK(viewer_rooms.rooms().rooms(2).name() == "Кормчая");
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
}


TEST_CASE("a host admits a dweller, regards him anew, and he sees his abode as his kind does")
{
	const std::uint16_t port = static_cast<std::uint16_t>(pick_port() + 1);
	const std::string directory = "/tmp/will-dwellers-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

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

	// The host looks at his abode: no one waits at his gates.
	send_turn_to_abode(*host.stream, {});
	CHECK(read_until(*host.stream, [](const auto& e) { return e.has_outstanding(); }).outstanding().waiting() == 0);
	CHECK(read_event(*host.stream).has_history_end());

	// Out through his own gates he comes to the host's: they are shut. Standing there, he
	// does not wait yet: the host, looking at his abode, sees no one waiting.
	stand_in(*man.stream, {}, "Врата");
	send_turn_to_abode(*man.stream, host_name, "Врата");
	CHECK(read_event(*host.stream).outstanding().waiting() == 0);
	const will::v1::ServerEvent at_gates = read_event(*man.stream);
	CHECK(at_gates.turned().aspect() == will::v1::THRESHOLD);
	const will::v1::ServerEvent shut = read_event(*man.stream);
	REQUIRE(shut.has_threshold());
	CHECK_FALSE(shut.threshold().open());
	CHECK_FALSE(shut.threshold().knocking());
	CHECK(read_event(*man.stream).has_history_end());
	send_admit(*host.stream, man_name);
	CHECK(read_event(*host.stream).has_protocol_notice());

	// He knocks: the host sees that one waits; the host is not there and cannot let him in.
	send_knock(*man.stream);
	CHECK(read_event(*man.stream).threshold().knocking());
	CHECK(read_event(*host.stream).outstanding().waiting() == 1);
	send_knock(*man.stream);
	CHECK(read_event(*man.stream).protocol_notice().code() == will::v1::NOTICE_ALREADY_KNOCKS);
	send_admit(*host.stream, man_name);
	CHECK(read_event(*host.stream).has_protocol_notice());

	// The host comes into his gates: he sees who waits, and the one waiting sees them open.
	send_turn_to_abode(*host.stream, {}, "Врата");
	CHECK(read_event(*host.stream).has_turned());
	const will::v1::ServerEvent waiting = read_event(*host.stream);
	REQUIRE(waiting.threshold().waiting_size() == 1);
	CHECK(waiting.threshold().waiting(0) == man_name);
	CHECK(read_event(*host.stream).has_unborn());  // keeping the gates, he sees the unborn too
	CHECK(read_event(*host.stream).has_history_end());
	CHECK(read_event(*man.stream).threshold().open());

	// He lets him in: the one let in is told he dwells there as an acquaintance; no one waits now.
	send_admit(*host.stream, man_name);
	CHECK(read_event(*host.stream).has_protocol_notice());
	CHECK(read_event(*host.stream).threshold().waiting_size() == 0);
	CHECK(read_event(*host.stream).has_unborn());
	const will::v1::ServerEvent dwelling = read_event(*man.stream);
	REQUIRE(dwelling.has_dwelling());
	CHECK(dwelling.dwelling().host_name() == host_name);
	CHECK(dwelling.dwelling().kind() == will::v1::ACQUAINTANCE);
	CHECK(read_event(*man.stream).has_threshold());

	send_turn_to_abode(*man.stream, host_name);
	CHECK(read_event(*host.stream).has_threshold());  // he left the gates the host stands in
	CHECK(read_event(*host.stream).has_unborn());
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
	send_turn_to_abode(*man.stream, host_name, "Слово");
	CHECK(read_event(*man.stream).has_protocol_notice());

	// Away from his helm the host may not regard him anew; at it he may. A friend is
	// shown at once every room but the helm, the host's alone: the cell and the gates.
	send_regard(*host.stream, man_name, will::v1::FRIEND);
	CHECK(read_event(*host.stream).has_protocol_notice());
	stand_in(*host.stream, {}, "Кормчая");
	send_regard(*host.stream, man_name, will::v1::FRIEND);
	CHECK(read_event(*host.stream).has_protocol_notice());
	const will::v1::ServerEvent helm = read_event(*host.stream);
	REQUIRE(helm.dwellers().dwellers_size() == 2);  // and the elder, his father by flesh
	for (const will::v1::Dweller& dweller : helm.dwellers().dwellers())
		CHECK(dweller.kind() == (dweller.name() == man_name ? will::v1::FRIEND : will::v1::NEIGHBOUR));
	CHECK(read_event(*host.stream).has_rooms());  // the host at his helm is told his rooms anew
	CHECK(read_event(*host.stream).has_supplications());
	CHECK(read_event(*man.stream).dwelling().kind() == will::v1::FRIEND);
	REQUIRE(read_event(*man.stream).turned().abode_of() == host_name);
	const will::v1::ServerEvent open = read_event(*man.stream);
	REQUIRE(open.rooms().rooms_size() == 2);
	CHECK(open.rooms().rooms(0).name() == "Слово");
	CHECK(read_event(*man.stream).has_history_end());

	// Entering the cell, he sees the words.
	send_turn_to_abode(*man.stream, host_name, "Слово");
	CHECK(read_event(*man.stream).turned().room() == "Слово");
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

	// Regarded as an acquaintance again, he no longer enters the cell: at once he withdraws
	// into the abode, and sees no room of it but the gates.
	stand_in(*host.stream, {}, "Кормчая");
	send_regard(*host.stream, man_name, will::v1::ACQUAINTANCE);
	CHECK(read_event(*host.stream).has_protocol_notice());
	CHECK(read_event(*host.stream).has_dwellers());
	CHECK(read_event(*host.stream).has_rooms());
	CHECK(read_event(*host.stream).has_supplications());
	CHECK(read_event(*man.stream).dwelling().kind() == will::v1::ACQUAINTANCE);
	const will::v1::ServerEvent withdrawn = read_event(*man.stream);
	REQUIRE(withdrawn.turned().abode_of() == host_name);
	CHECK(withdrawn.turned().room().empty());
	const will::v1::ServerEvent gates_only = read_event(*man.stream);
	REQUIRE(gates_only.rooms().rooms_size() == 1);
	CHECK(gates_only.rooms().rooms(0).name() == "Врата");
	CHECK(read_event(*man.stream).has_history_end());

	// Away from his helm the host does not arrange his rooms.
	stand_in(*host.stream, {}, {});
	{
		will::v1::ClientEvent arrange;
		arrange.mutable_arrange()->set_room("Слово");
		arrange.mutable_arrange()->set_part(will::v1::OUTER);
		REQUIRE(host.stream->Write(arrange));
	}
	CHECK(read_event(*host.stream).protocol_notice().message() == "one does this only standing at a helm");

	// In it he sees his dwellers and all his rooms, each in its part.
	send_turn_to_abode(*host.stream, {}, "Кормчая");
	CHECK(read_event(*host.stream).has_turned());
	CHECK(read_event(*host.stream).has_dwellers());
	const will::v1::ServerEvent shown_rooms = read_event(*host.stream);
	REQUIRE(shown_rooms.rooms().rooms_size() == 3);
	CHECK(shown_rooms.rooms().rooms(0).part() == will::v1::INNER);
	CHECK(read_event(*host.stream).has_supplications());
	CHECK(read_event(*host.stream).has_history_end());

	// The host sets his cell in the outer part; a neighbour, in the abode, sees it there, enters
	// and sees the words again.
	send_regard(*host.stream, man_name, will::v1::NEIGHBOUR);
	CHECK(read_event(*host.stream).has_protocol_notice());
	CHECK(read_event(*host.stream).has_dwellers());
	CHECK(read_event(*host.stream).has_rooms());
	CHECK(read_event(*host.stream).has_supplications());
	CHECK(read_event(*man.stream).dwelling().kind() == will::v1::NEIGHBOUR);
	REQUIRE(read_event(*man.stream).turned().room().empty());
	CHECK(read_event(*man.stream).rooms().rooms_size() == 1);  // the gates alone
	CHECK(read_event(*man.stream).has_history_end());
	{
		will::v1::ClientEvent arrange;
		arrange.mutable_arrange()->set_room("Слово");
		arrange.mutable_arrange()->set_part(will::v1::OUTER);
		REQUIRE(host.stream->Write(arrange));
	}
	CHECK(read_event(*host.stream).has_protocol_notice());
	CHECK(read_event(*host.stream).has_dwellers());  // his helm is told anew to all at it
	const will::v1::ServerEvent arranged = read_event(*host.stream);  // all his rooms, each in its part
	REQUIRE(arranged.rooms().rooms_size() == 3);
	CHECK(arranged.rooms().rooms(0).name() == "Слово");
	CHECK(arranged.rooms().rooms(0).part() == will::v1::OUTER);
	CHECK(read_event(*host.stream).has_supplications());
	REQUIRE(read_event(*man.stream).turned().room().empty());
	const will::v1::ServerEvent opened = read_event(*man.stream);
	REQUIRE(opened.rooms().rooms_size() == 2);
	CHECK(opened.rooms().rooms(0).name() == "Слово");
	CHECK(read_event(*man.stream).has_history_end());
	send_turn_to_abode(*man.stream, host_name, "Слово");
	CHECK(read_event(*man.stream).turned().room() == "Слово");
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
}


TEST_CASE("one lists the abodes one dwells in and the supplications awaiting one, and rejects one")
{
	const std::uint16_t port = static_cast<std::uint16_t>(pick_port() + 2);
	const std::string directory = "/tmp/will-lists-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

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
	knock_at_gates(*b.stream, a_name);
	send_admit(*a.stream, b_name);
	CHECK(next_past_gates(*a.stream).has_protocol_notice());
	CHECK(next_past_gates(*b.stream).has_dwelling());
	stand_in(*b.stream, {}, "Врата");
	knock_at_gates(*a.stream, b_name);
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

	// b supplicates a, standing at his helm; a sees it awaiting and rejects it.
	stand_in(*b.stream, {}, "Кормчая");
	{
		will::v1::ClientEvent ask;
		ask.mutable_supplicate()->set_addressee_name(a_name);
		REQUIRE(b.stream->Write(ask));
	}
	CHECK(next_past_gates(*b.stream).has_protocol_notice());
	CHECK(next_past_gates(*a.stream).has_supplication_offer());
	// His helm is told anew to him; then he turns to his word.
	read_until(*b.stream, [](const auto& e) { return e.has_supplications(); });
	stand_in(*b.stream, {}, "Слово");

	// Each sees, among his dwellers, how the other stands to him: b asked, a is asked.
	const auto dweller_named = [](SessionStream& stream, const std::string& name) {
		will::v1::ClientEvent list;
		list.mutable_list_dwellers();
		REQUIRE(stream.Write(list));
		const will::v1::ServerEvent told = read_until(stream, [](const auto& e) { return e.has_dwellers(); });
		for (const will::v1::Dweller& dweller : told.dwellers().dwellers()) {
			if (dweller.name() == name)
				return dweller;
		}
		FAIL("no such dweller");
		return will::v1::Dweller{};
	};
	CHECK(dweller_named(*a.stream, b_name).asks());
	CHECK_FALSE(dweller_named(*a.stream, b_name).asked());
	CHECK(dweller_named(*b.stream, a_name).asked());

	// Away from his helm a does not answer; coming home through his gates to it,
	// he sees who asks him, and rejects him there.
	{
		will::v1::ClientEvent reject;
		reject.mutable_reject_supplication()->set_suppliant_name(b_name);
		REQUIRE(a.stream->Write(reject));
	}
	CHECK(next_past_gates(*a.stream).protocol_notice().code() == will::v1::NOTICE_STAND_AT_HELM);
	stand_in(*a.stream, {}, "Врата");
	send_turn_to_abode(*a.stream, {}, "Кормчая");
	const will::v1::ServerEvent awaiting = read_until(*a.stream, [](const auto& e) { return e.has_supplications(); });
	REQUIRE(awaiting.supplications().suppliant_names_size() == 1);
	CHECK(awaiting.supplications().suppliant_names(0) == b_name);
	CHECK(read_event(*a.stream).has_history_end());
	{
		will::v1::ClientEvent reject;
		reject.mutable_reject_supplication()->set_suppliant_name(b_name);
		REQUIRE(a.stream->Write(reject));
	}
	CHECK(next_past_gates(*a.stream).has_protocol_notice());
	CHECK(read_until(*a.stream, [](const auto& e) { return e.has_supplications(); }).supplications().suppliant_names_size() == 0);
	{
		will::v1::ClientEvent list;
		list.mutable_list_supplications();
		REQUIRE(a.stream->Write(list));
	}
	CHECK(next_past_gates(*a.stream).supplications().suppliant_names_size() == 0);
	CHECK_FALSE(dweller_named(*a.stream, b_name).asks());
	// The suppliant is told he was declined, and by whom.
	const will::v1::ServerEvent declined = read_until(*b.stream, [](const auto& e) {
		return e.protocol_notice().code() == will::v1::NOTICE_SUPPLICATION_DECLINED;
	});
	REQUIRE(declined.protocol_notice().args_size() == 1);
	CHECK(declined.protocol_notice().args(0) == a_name);
	CHECK_FALSE(dweller_named(*b.stream, a_name).asked());

	// b asks for rooms without moving his gaze: his own three; in a's abode, as an acquaintance, only the gates.
	{
		will::v1::ClientEvent list;
		list.mutable_list_rooms();
		REQUIRE(b.stream->Write(list));
	}
	const will::v1::ServerEvent own_rooms = next_past_gates(*b.stream);
	REQUIRE(own_rooms.has_rooms());
	REQUIRE(own_rooms.rooms().rooms_size() == 3);
	CHECK(own_rooms.rooms().rooms(0).name() == "Слово");
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
}


TEST_CASE("an unborn body waits, seen by the keepers of gates; it is born of their host; the father by spirit is not yet open")
{
	const std::uint16_t port = static_cast<std::uint16_t>(pick_port() + 3);
	const std::string directory = "/tmp/will-birth-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

	RunningServer server{start_server(g_server_exe, port, directory)};
	REQUIRE(server.pid() > 0);
	wait_for_server(port);

	// The first comes into the world at once.
	Elder elder = first_man(port);
	SessionStream& adam = *elder.session.stream;

	// Standing at his gates, he keeps them and sees no one awaiting birth.
	send_turn_to_abode(adam, {}, "Врата");
	CHECK(read_event(adam).turned().aspect() == will::v1::THRESHOLD);
	CHECK(read_event(adam).threshold().keeping());
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
	// The welcome tells the server's present instant, that the client set its clock at once.
	{
		const std::int64_t now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
										std::chrono::system_clock::now().time_since_epoch())
										.count();
		CHECK(std::llabs(welcomed.auth_ok().server_time_ns() - now_ns) < 5'000'000'000LL);
	}
	CHECK(welcomed.auth_ok().name().empty());
	const std::uint64_t mark = welcomed.auth_ok().mark();
	send_turn_to_abode(*seth.stream, {}, "Слово");
	CHECK(read_event(*seth.stream).protocol_notice().message() == "one awaits one's birth");
	// Awaiting, his heart beats and is heard.
	{
		will::v1::ClientEvent beat;
		beat.mutable_heartbeat();
		beat.set_request_id(1);
		REQUIRE(seth.stream->Write(beat));
	}
	const will::v1::ServerEvent heard = read_event(*seth.stream);
	REQUIRE(heard.has_reply());
	CHECK(heard.reply().granted());
	CHECK(heard.reply().notice().empty());

	// Adam sees him awaiting at once, and bears him.
	const will::v1::ServerEvent awaiting = read_event(adam);
	REQUIRE(awaiting.unborn().marks_size() == 1);
	CHECK(awaiting.unborn().marks(0) == mark);
	send_bear(adam, mark);
	const will::v1::ServerEvent born_told = read_event(adam);
	const std::string born_notice = born_told.protocol_notice().message();
	CHECK(read_event(adam).has_dwelling());
	CHECK(read_event(adam).unborn().marks_size() == 0);

	const will::v1::ServerEvent born = read_event(*seth.stream);
	REQUIRE(born.has_auth_ok());
	const std::string seth_name = born.auth_ok().name();
	CHECK(born.auth_ok().father() == elder.name);  // told who gave the name, as he is born
	CHECK(born_notice == seth_name + " born");
	// What was done is told by its code, the name in it apart.
	CHECK(born_told.protocol_notice().code() == will::v1::NOTICE_BORN);
	REQUIRE(born_told.protocol_notice().args_size() == 1);
	CHECK(born_told.protocol_notice().args(0) == seth_name);
	const will::v1::ServerEvent dwelling = read_event(*seth.stream);
	CHECK(dwelling.dwelling().host_name() == elder.name);
	CHECK(dwelling.dwelling().kind() == will::v1::ACQUAINTANCE);

	// Many stand at the gates at once. Seth comes to Adam's gates beside him: he only
	// waits there, does not keep them, and is not shown the unborn.
	stand_in(*seth.stream, {}, "Врата");
	send_turn_to_abode(*seth.stream, elder.name, "Врата");
	CHECK(read_event(*seth.stream).turned().aspect() == will::v1::THRESHOLD);
	CHECK_FALSE(read_event(*seth.stream).threshold().keeping());
	CHECK(read_event(*seth.stream).has_history_end());

	// Another body comes: Adam sees it, Seth does not, nor may he bear it.
	GrpcSession enos = open_session(port);
	REQUIRE(enos.stream);
	{
		will::v1::ClientEvent bind;
		bind.mutable_bind_token()->set_token(ViewerToken);
		REQUIRE(enos.stream->Write(bind));
	}
	const std::uint64_t enos_mark = read_event(*enos.stream).auth_ok().mark();
	const will::v1::ServerEvent seen = read_until(adam, [](const auto& e) { return e.unborn().marks_size() > 0; });
	CHECK(seen.unborn().marks(0) == enos_mark);
	send_bear(*seth.stream, enos_mark);
	const will::v1::ServerEvent not_keeper = read_event(*seth.stream);
	CHECK(not_keeper.protocol_notice().message() == "only the host acts here; others but look");
	// The world's refusal is told by its reason's code, beside its words.
	CHECK(not_keeper.protocol_notice().code() == will::v1::NOTICE_ONLY_HOST_ACTS);

	// The father by spirit is not yet open: neither choosing him nor the line is done.
	{
		will::v1::ClientEvent choose;
		choose.mutable_choose_father()->set_name(elder.name);
		REQUIRE(seth.stream->Write(choose));
	}
	CHECK(read_event(*seth.stream).protocol_notice().message() == "the father by spirit is not yet open");
	{
		will::v1::ClientEvent list;
		list.mutable_list_lineage();
		REQUIRE(adam.Write(list));
	}
	CHECK(read_until(adam, [](const auto& e) { return e.has_protocol_notice(); }).protocol_notice().message()
		  == "the father by spirit is not yet open");

	enos.context->TryCancel();
	seth.context->TryCancel();
	elder.session.context->TryCancel();
	server.stop();
}


TEST_CASE("a friend standing in the host's gates lets no one in: only the host keeps them")
{
	const std::uint16_t port = static_cast<std::uint16_t>(pick_port() + 4);
	const std::string directory = "/tmp/will-keeper-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

	RunningServer server{start_server(g_server_exe, port, directory)};
	REQUIRE(server.pid() > 0);
	wait_for_server(port);

	Elder elder = first_man(port);
	GrpcSession anna = open_session(port);
	GrpcSession boris = open_session(port);
	GrpcSession viktor = open_session(port);
	const std::string anna_name = born_named(elder, *anna.stream, SenderToken);
	const std::string boris_name = born_named(elder, *boris.stream, ViewerToken);
	const std::string viktor_name = born_named(elder, *viktor.stream, "33333333333333333333333333333333");

	// Anna lets Boris in and regards him as a friend; she stays at her helm.
	stand_in(*anna.stream, {}, "Врата");
	knock_at_gates(*boris.stream, anna_name);
	send_admit(*anna.stream, boris_name);
	read_until(*anna.stream, [](const auto& e) { return e.has_protocol_notice(); });
	read_until(*boris.stream, [](const auto& e) { return e.has_dwelling(); });
	stand_in(*anna.stream, {}, "Кормчая");
	send_regard(*anna.stream, boris_name, will::v1::FRIEND);
	read_until(*anna.stream, [](const auto& e) { return e.has_supplications(); });  // her helm told anew
	CHECK(read_until(*boris.stream, [](const auto& e) { return e.has_dwelling(); }).dwelling().kind()
		  == will::v1::FRIEND);

	// A friend, Boris still does not keep her gates: he lets Viktor in no more than before.
	knock_at_gates(*viktor.stream, anna_name);
	send_admit(*boris.stream, viktor_name);
	CHECK(read_until(*boris.stream, [](const auto& e) { return e.has_protocol_notice(); }).protocol_notice().code()
		  == will::v1::NOTICE_ONLY_HOST_ACTS);

	// Anna, standing in her gates, lets Viktor in.
	stand_in(*anna.stream, {}, "Врата");
	send_admit(*anna.stream, viktor_name);
	CHECK(read_until(*anna.stream, [](const auto& e) { return e.has_protocol_notice(); }).protocol_notice().message()
		  == viktor_name + " admitted as an acquaintance");
	const will::v1::ServerEvent dwelling = read_until(*viktor.stream, [](const auto& e) { return e.has_dwelling(); });
	CHECK(dwelling.dwelling().host_name() == anna_name);
	CHECK(dwelling.dwelling().kind() == will::v1::ACQUAINTANCE);
	send_turn_to_abode(*anna.stream, {}, "Кормчая");
	const will::v1::ServerEvent upper = read_until(*anna.stream, [](const auto& e) { return e.has_dwellers(); });
	bool seen = false;
	for (const will::v1::Dweller& dweller : upper.dwellers().dwellers())
		seen = seen || (dweller.name() == viktor_name && dweller.kind() == will::v1::ACQUAINTANCE);
	CHECK(seen);

	anna.context->TryCancel();
	boris.context->TryCancel();
	viktor.context->TryCancel();
	elder.session.context->TryCancel();
	server.stop();
}


TEST_CASE("the testator wills a training over the wire; the novice fulfils it telling what he has done")
{
	const std::uint16_t port = static_cast<std::uint16_t>(pick_port() + 5);
	const std::string directory = "/tmp/will-training-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

	RunningServer server{start_server(g_server_exe, port, directory)};
	REQUIRE(server.pid() > 0);
	wait_for_server(port);

	Elder elder = first_man(port);
	GrpcSession trainer = open_session(port);
	GrpcSession novice = open_session(port);
	const std::string trainer_name = born_named(elder, *trainer.stream, SenderToken);
	const std::string novice_name = born_named(elder, *novice.stream, ViewerToken);

	// They let each other in, and the novice asks the trainer, who accepts.
	stand_in(*trainer.stream, {}, "Врата");
	knock_at_gates(*novice.stream, trainer_name);
	send_admit(*trainer.stream, novice_name);
	read_until(*trainer.stream, [](const auto& e) { return e.has_protocol_notice(); });
	read_until(*novice.stream, [](const auto& e) { return e.has_dwelling(); });
	stand_in(*novice.stream, {}, "Врата");
	knock_at_gates(*trainer.stream, novice_name);
	send_admit(*novice.stream, trainer_name);
	read_until(*novice.stream, [](const auto& e) { return e.has_protocol_notice(); });
	read_until(*trainer.stream, [](const auto& e) { return e.has_dwelling(); });
	stand_in(*novice.stream, {}, "Кормчая");
	{
		will::v1::ClientEvent ask;
		ask.mutable_supplicate()->set_addressee_name(trainer_name);
		REQUIRE(novice.stream->Write(ask));
	}
	read_until(*trainer.stream, [](const auto& e) { return e.has_supplication_offer(); });
	stand_in(*trainer.stream, {}, "Врата");
	stand_in(*trainer.stream, {}, "Кормчая");
	{
		will::v1::ClientEvent accept;
		accept.mutable_accept_supplication()->set_suppliant_name(novice_name);
		REQUIRE(trainer.stream->Write(accept));
	}
	read_until(*trainer.stream, [](const auto& e) { return e.has_tie_formed(); });
	read_until(*novice.stream, [](const auto& e) { return e.has_tie_formed(); });

	// Each sees, among his dwellers, how the other stands to him in the tie.
	for (const auto& [stream, other, trains] : {std::tuple{trainer.stream.get(), novice_name, false},
												 std::tuple{novice.stream.get(), trainer_name, true}}) {
		will::v1::ClientEvent list;
		list.mutable_list_dwellers();
		REQUIRE(stream->Write(list));
		const will::v1::ServerEvent told = read_until(*stream, [](const auto& e) { return e.has_dwellers(); });
		bool seen = false;
		for (const will::v1::Dweller& dweller : told.dwellers().dwellers()) {
			if (dweller.name() != other)
				continue;
			seen = true;
			CHECK(dweller.trainer() == trains);
			CHECK(dweller.novice() == !trains);
			CHECK_FALSE(dweller.asked());
			CHECK_FALSE(dweller.asks());
		}
		CHECK(seen);
	}

	// Each enters his room of the tie.
	send_turn_to_abode(*trainer.stream, {}, "Ведение — " + novice_name);
	read_until(*trainer.stream, [](const auto& e) { return e.has_history_end(); });
	send_turn_to_abode(*novice.stream, {}, "Послушание — " + trainer_name);
	read_until(*novice.stream, [](const auto& e) { return e.has_history_end(); });

	// The novice does not will a training; the trainer does.
	will::v1::ClientEvent train;
	train.mutable_train()->set_title("Понедельник");
	{
		will::v1::Exercise* press = train.mutable_train()->add_exercises();
		press->set_name("Жим лёжа");
		press->set_instruction("Лопатки сведены");
		for (const auto [grams, reps] : {std::pair{60'000u, 10u}, {70'000u, 8u}}) {
			will::v1::Approach* go = press->add_approaches();
			go->set_weight_grams(grams);
			go->set_repetitions(reps);
		}
	}
	REQUIRE(novice.stream->Write(train));
	const will::v1::ServerEvent not_testator
		= read_until(*novice.stream, [](const auto& e) { return e.has_protocol_notice(); });
	CHECK(not_testator.protocol_notice().message() == "only the testator of a tie wills in it");
	CHECK(not_testator.protocol_notice().code() == will::v1::NOTICE_ONLY_TESTATOR_WILLS);
	REQUIRE(trainer.stream->Write(train));
	CHECK(read_until(*trainer.stream, [](const auto& e) { return e.has_protocol_notice(); })
			  .protocol_notice()
			  .message()
			  .ends_with(" willed"));

	const will::v1::ServerEvent willed = read_until(*novice.stream, [](const auto& e) { return e.has_word(); });
	CHECK(willed.word().kind() == will::v1::Word::BEHEST);
	CHECK(willed.word().body() == "Понедельник");
	REQUIRE(willed.word().exercises_size() == 1);
	CHECK(willed.word().exercises(0).name() == "Жим лёжа");
	CHECK(willed.word().exercises(0).instruction() == "Лопатки сведены");
	REQUIRE(willed.word().exercises(0).approaches_size() == 2);
	CHECK(willed.word().exercises(0).approaches(1).weight_grams() == 70'000);

	// He does it approach by approach: the trainer sees each begun and each done.
	const std::uint64_t id = willed.word().id();
	for (const auto [approach, reps] : {std::pair{0u, 10u}, {1u, 6u}}) {
		will::v1::ClientEvent begin;
		begin.mutable_begin_approach()->set_behest_id(id);
		begin.mutable_begin_approach()->set_exercise(0);
		begin.mutable_begin_approach()->set_approach(approach);
		REQUIRE(novice.stream->Write(begin));
		const will::v1::ServerEvent underway
			= read_until(*trainer.stream, [](const auto& e) { return e.has_underway() && e.underway().doing(); });
		CHECK(underway.underway().behest_id() == id);
		CHECK(underway.underway().approach() == approach);

		// Looking at his abode, the novice sees among what awaits him the approach underway.
		if (approach == 0) {
			read_until(*novice.stream, [](const auto& e) { return e.has_underway() && e.underway().doing(); });
			send_turn_to_abode(*novice.stream, {}, {});
			const will::v1::ServerEvent overview
				= read_until(*novice.stream, [](const auto& e) { return e.has_outstanding(); });
			REQUIRE(overview.outstanding().owed_size() == 1);
			CHECK(overview.outstanding().owed(0).counterpart() == trainer_name);
			CHECK(overview.outstanding().owed(0).underway().behest_id() == id);
			CHECK(overview.outstanding().owed(0).underway().doing());
			CHECK(overview.outstanding().owed(0).underway().approach() == 0);
			read_until(*novice.stream, [](const auto& e) { return e.has_history_end(); });
			// The room of the tie tells the novice whose it is: the trainer teaches there; the trainer's does not.
			const std::string obedience = overview.outstanding().owed(0).room();
			send_turn_to_abode(*novice.stream, {}, obedience);
			CHECK(read_until(*novice.stream, [](const auto& e) { return e.has_turned(); }).turned().testator() == trainer_name);
			read_until(*novice.stream, [](const auto& e) { return e.has_history_end(); });

			// And the trainer, looking at his, sees it among what he set, with whom he set it for.
			send_turn_to_abode(*trainer.stream, {}, {});
			const will::v1::ServerEvent set
				= read_until(*trainer.stream, [](const auto& e) { return e.has_outstanding(); });
			REQUIRE(set.outstanding().awaited_size() == 1);
			CHECK(set.outstanding().awaited(0).counterpart() == novice_name);
			read_until(*trainer.stream, [](const auto& e) { return e.has_history_end(); });
			send_turn_to_abode(*trainer.stream, {}, set.outstanding().awaited(0).room());
			CHECK(read_until(*trainer.stream, [](const auto& e) { return e.has_turned(); }).turned().testator().empty());
			read_until(*trainer.stream, [](const auto& e) { return e.has_history_end(); });
		}

		// The second approach is ended first and told after: it finished when it ended.
		std::int64_t ended_at = 0;
		if (approach == 1) {
			will::v1::ClientEvent end;
			end.mutable_end_approach()->set_behest_id(id);
			REQUIRE(novice.stream->Write(end));
			const will::v1::ServerEvent ended
				= read_until(*trainer.stream, [](const auto& e) { return e.has_underway() && e.underway().ended_at_ns() != 0; });
			ended_at = ended.underway().ended_at_ns();
			CHECK(ended_at >= ended.underway().begun_at_ns());
			read_until(*novice.stream, [](const auto& e) { return e.has_underway() && e.underway().ended_at_ns() != 0; });

			will::v1::ClientEvent again;
			again.mutable_end_approach()->set_behest_id(id);
			REQUIRE(novice.stream->Write(again));
			CHECK(read_until(*novice.stream, [](const auto& e) { return e.has_protocol_notice(); }).protocol_notice().code()
				  == will::v1::NOTICE_TELL_WHAT_WAS_DONE);
		}

		will::v1::ClientEvent finish;
		finish.mutable_finish_approach()->set_behest_id(id);
		finish.mutable_finish_approach()->set_weight_grams(70'000);
		finish.mutable_finish_approach()->set_repetitions(reps);
		finish.mutable_finish_approach()->set_strain(approach);
		REQUIRE(novice.stream->Write(finish));
		const will::v1::ServerEvent exerted = read_until(*trainer.stream, [](const auto& e) { return e.has_exerted(); });
		CHECK(exerted.exerted().behest_id() == id);
		CHECK(exerted.exerted().effort().approach() == approach);
		CHECK(exerted.exerted().effort().repetitions() == reps);
		CHECK(exerted.exerted().effort().finished_at_ns() >= exerted.exerted().effort().begun_at_ns());
		if (ended_at != 0)
			CHECK(exerted.exerted().effort().finished_at_ns() == ended_at);
		CHECK(exerted.exerted().effort().strain() == approach);
		CHECK(read_until(*novice.stream, [](const auto& e) { return e.has_exerted(); }).exerted().effort().approach()
			  == approach);
	}

	// The same approach is not done twice.
	{
		will::v1::ClientEvent again;
		again.mutable_begin_approach()->set_behest_id(id);
		again.mutable_begin_approach()->set_exercise(0);
		again.mutable_begin_approach()->set_approach(0);
		REQUIRE(novice.stream->Write(again));
		CHECK(read_until(*novice.stream, [](const auto& e) { return e.has_protocol_notice(); }).protocol_notice().message()
			  == "the approach is already done");
	}

	// He remarks on the exercise: the trainer is told at once, and the training keeps it.
	{
		will::v1::ClientEvent say;
		say.mutable_say_remark()->set_behest_id(id);
		say.mutable_say_remark()->set_exercise(0);
		say.mutable_say_remark()->set_body("Последний тяжело");
		REQUIRE(novice.stream->Write(say));
		const will::v1::ServerEvent remarked = read_until(*trainer.stream, [](const auto& e) { return e.has_remarked(); });
		CHECK(remarked.remarked().behest_id() == id);
		CHECK(remarked.remarked().remark().exercise() == 0);
		CHECK(remarked.remarked().remark().body() == "Последний тяжело");
		CHECK(remarked.remarked().remark().said_at_ns() > 0);
		read_until(*novice.stream, [](const auto& e) { return e.has_remarked(); });

		say.mutable_say_remark()->set_exercise(5);
		REQUIRE(novice.stream->Write(say));
		CHECK(read_until(*novice.stream, [](const auto& e) { return e.has_protocol_notice(); }).protocol_notice().code()
			  == will::v1::NOTICE_NO_SUCH_EXERCISE);
	}

	// He finishes the training while the trainer looks at his abode.
	send_turn_to_abode(*trainer.stream, {}, {});
	const std::string shepherding_room
		= read_until(*trainer.stream, [](const auto& e) { return e.has_outstanding(); }).outstanding().awaited(0).room();
	read_until(*trainer.stream, [](const auto& e) { return e.has_history_end(); });
	will::v1::ClientEvent fulfil;
	fulfil.mutable_fulfil()->set_behest_id(id);
	REQUIRE(novice.stream->Write(fulfil));
	CHECK(read_until(*novice.stream, [](const auto& e) { return e.has_protocol_notice(); })
			  .protocol_notice()
			  .message()
			  .ends_with(" fulfilled"));

	// Looking at his abode, the trainer is told it anew: not seen by him, the deed awaits his review,
	// with whose it is; seen in his room, it awaits no more.
	const will::v1::ServerEvent unseen = read_until(*trainer.stream, [](const auto& e) { return e.has_outstanding(); });
	CHECK(unseen.outstanding().awaited_size() == 0);
	REQUIRE(unseen.outstanding().unreviewed_size() == 1);
	CHECK(unseen.outstanding().unreviewed(0).behest().id() == id);
	CHECK(unseen.outstanding().unreviewed(0).counterpart() == novice_name);
	CHECK(unseen.outstanding().unreviewed(0).room() == shepherding_room);
	// And he hears of it in a word: the deed itself, done in his room of the tie.
	const will::v1::ServerEvent stirred = read_until(*trainer.stream, [](const auto& e) { return e.has_stirred(); });
	CHECK(stirred.stirred().author_name() == novice_name);
	CHECK(stirred.stirred().room() == shepherding_room);
	CHECK(stirred.stirred().word().kind() == will::v1::Word::DEED);
	CHECK(stirred.stirred().word().behest_id() == id);
	send_turn_to_abode(*trainer.stream, {}, shepherding_room);
	const will::v1::ServerEvent done
		= read_until(*trainer.stream, [](const auto& e) { return e.has_word() && e.word().kind() == will::v1::Word::DEED; });
	CHECK(done.word().behest_id() == id);
	REQUIRE(done.word().exercises_size() == 1);
	REQUIRE(done.word().exercises(0).approaches_size() == 2);
	CHECK(done.word().exercises(0).approaches(1).repetitions() == 6);
	read_until(*trainer.stream, [](const auto& e) { return e.has_history_end(); });
	send_turn_to_abode(*trainer.stream, {}, {});
	CHECK(read_until(*trainer.stream, [](const auto& e) { return e.has_outstanding(); }).outstanding().unreviewed_size()
		  == 0);

	trainer.context->TryCancel();
	novice.context->TryCancel();
	elder.session.context->TryCancel();
	server.stop();
}


TEST_CASE("a numbered request is answered once, by its number: granted or refused, with what is said of it")
{
	const std::uint16_t port = static_cast<std::uint16_t>(pick_port() + 9);
	const std::string directory = "/tmp/will-reply-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

	RunningServer server{start_server(g_server_exe, port, directory)};
	REQUIRE(server.pid() > 0);
	wait_for_server(port);

	Elder elder = first_man(port);
	SessionStream& adam = *elder.session.stream;

	// Refused: the answer says so, and nothing is told besides.
	{
		will::v1::ClientEvent turn;
		turn.mutable_turn()->set_room("Нет такой");
		turn.set_request_id(7);
		REQUIRE(adam.Write(turn));
	}
	will::v1::ServerEvent refused;
	REQUIRE(adam.Read(&refused));
	REQUIRE(refused.has_reply());
	CHECK(refused.reply().request_id() == 7);
	CHECK_FALSE(refused.reply().granted());
	CHECK(refused.reply().notice() == "no such room");
	CHECK(refused.reply().code() == will::v1::NOTICE_NO_SUCH_ROOM);

	// Granted: what it brings comes first, then the answer, with what is said of it.
	{
		will::v1::ClientEvent turn;
		turn.mutable_turn()->set_room("Слово");
		turn.set_request_id(8);
		REQUIRE(adam.Write(turn));
	}
	const will::v1::ServerEvent granted = read_until(adam, [](const auto& e) { return e.has_reply(); });
	CHECK(granted.reply().request_id() == 8);
	CHECK(granted.reply().granted());

	// Unnumbered, as before: a notice.
	send_turn_to_abode(adam, {}, "Нет такой");
	CHECK(read_event(adam).protocol_notice().message() == "no such room");

	// Naming his own abode, he is told it unnamed: it is his own.
	send_turn_to_abode(adam, elder.name, "Врата");
	const will::v1::ServerEvent own_gates = read_event(adam);
	CHECK(own_gates.turned().abode_of().empty());
	CHECK(own_gates.turned().room() == "Врата");
	read_until(adam, [](const auto& e) { return e.has_history_end(); });

	// A heartbeat is answered by its number alone, granted.
	{
		will::v1::ClientEvent beat;
		beat.mutable_heartbeat();
		beat.set_request_id(9);
		REQUIRE(adam.Write(beat));
	}
	const will::v1::ServerEvent heard = read_event(adam);
	REQUIRE(heard.has_reply());
	CHECK(heard.reply().request_id() == 9);
	CHECK(heard.reply().granted());
	CHECK(heard.reply().notice().empty());

	elder.session.context->TryCancel();
	server.stop();
}


TEST_CASE("a training goes on across a restart of the server: the approach underway outlives it, begun when it was")
{
	const std::uint16_t port = static_cast<std::uint16_t>(pick_port() + 11);
	const std::string directory = "/tmp/will-restart-training-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

	std::optional<RunningServer> server{std::in_place, start_server(g_server_exe, port, directory)};
	REQUIRE(server->pid() > 0);
	wait_for_server(port);

	// The elder trains his child: the child, dwelling in his abode, asks him at her helm; he accepts at his.
	Elder elder = first_man(port);
	GrpcSession child = open_session(port);
	const std::string child_name = born_named(elder, *child.stream, SenderToken);
	const auto look = [](SessionStream& stream, const std::string& room) {
		send_turn_to_abode(stream, {}, room);
		read_until(stream, [](const auto& e) { return e.has_history_end(); });
	};
	look(*child.stream, "Кормчая");
	{
		will::v1::ClientEvent ask;
		ask.mutable_supplicate()->set_addressee_name(elder.name);
		REQUIRE(child.stream->Write(ask));
	}
	read_until(*elder.session.stream, [](const auto& e) { return e.has_supplication_offer(); });
	look(*elder.session.stream, "Кормчая");
	{
		will::v1::ClientEvent accept;
		accept.mutable_accept_supplication()->set_suppliant_name(child_name);
		REQUIRE(elder.session.stream->Write(accept));
	}
	read_until(*elder.session.stream, [](const auto& e) { return e.has_tie_formed(); });
	read_until(*child.stream, [](const auto& e) { return e.has_tie_formed(); });
	const std::string shepherding = "Ведение — " + child_name;
	const std::string obedience = "Послушание — " + elder.name;
	look(*elder.session.stream, shepherding);
	look(*child.stream, obedience);

	// He wills a training of two approaches; she begins the first.
	{
		will::v1::ClientEvent train;
		train.mutable_train()->set_title("Среда");
		will::v1::Exercise* squat = train.mutable_train()->add_exercises();
		squat->set_name("Присед");
		for (int i = 0; i < 2; ++i) {
			will::v1::Approach* go = squat->add_approaches();
			go->set_weight_grams(40'000);
			go->set_repetitions(5);
		}
		REQUIRE(elder.session.stream->Write(train));
	}
	const std::uint64_t id = read_until(*child.stream, [](const auto& e) { return e.has_word(); }).word().id();
	const auto begin = [&](SessionStream& stream, const std::uint32_t approach) {
		will::v1::ClientEvent go;
		go.mutable_begin_approach()->set_behest_id(id);
		go.mutable_begin_approach()->set_approach(approach);
		REQUIRE(stream.Write(go));
	};
	begin(*child.stream, 0);
	const std::int64_t begun
		= read_until(*child.stream, [](const auto& e) { return e.has_underway() && e.underway().doing(); }).underway().begun_at_ns();

	// The server goes down in the middle of the approach and comes up again on the same database.
	elder.session.context->TryCancel();
	child.context->TryCancel();
	server.reset();
	server.emplace(start_server(g_server_exe, port, directory));
	REQUIRE(server->pid() > 0);
	wait_for_server(port);

	GrpcSession trainer = open_session(port);
	GrpcSession novice = open_session(port);
	CHECK(bind_named(*trainer.stream, ElderToken) == elder.name);
	CHECK(bind_named(*novice.stream, SenderToken) == child_name);

	// Waking at home, she sees in what awaits her the approach still going, begun when it was begun.
	send_history_request(*novice.stream, 10);
	const will::v1::ServerEvent overview = read_until(*novice.stream, [](const auto& e) { return e.has_outstanding(); });
	REQUIRE(overview.outstanding().owed_size() == 1);
	CHECK(overview.outstanding().owed(0).underway().doing());
	CHECK(overview.outstanding().owed(0).underway().begun_at_ns() == begun);
	read_until(*novice.stream, [](const auto& e) { return e.has_history_end(); });

	// Back in the room of the tie, she tells the approach: it lasted from its true beginning.
	look(*trainer.stream, shepherding);
	look(*novice.stream, obedience);
	const auto finish = [&](const std::uint32_t reps) {
		will::v1::ClientEvent done;
		done.mutable_finish_approach()->set_behest_id(id);
		done.mutable_finish_approach()->set_weight_grams(40'000);
		done.mutable_finish_approach()->set_repetitions(reps);
		REQUIRE(novice.stream->Write(done));
		return read_until(*trainer.stream, [](const auto& e) { return e.has_exerted(); }).exerted().effort();
	};
	const will::v1::Effort first = finish(5);
	CHECK(first.approach() == 0);
	CHECK(first.begun_at_ns() == begun);
	CHECK(first.finished_at_ns() > begun);

	// The training goes on as if nothing had happened: the second approach, and the deed.
	begin(*novice.stream, 1);
	CHECK(finish(4).approach() == 1);
	{
		will::v1::ClientEvent fulfil;
		fulfil.mutable_fulfil()->set_behest_id(id);
		REQUIRE(novice.stream->Write(fulfil));
	}
	const will::v1::ServerEvent deed = read_until(*trainer.stream, [](const auto& e) {
		return e.has_word() && e.word().kind() == will::v1::Word::DEED;
	});
	CHECK(deed.word().behest_id() == id);

	trainer.context->TryCancel();
	novice.context->TryCancel();
	server->stop();
}


TEST_CASE("one who does not read does not hold up one who writes")
{
	const std::uint16_t port = static_cast<std::uint16_t>(pick_port() + 10);
	const std::string directory = "/tmp/will-slow-reader-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

	RunningServer server{start_server(g_server_exe, port, directory)};
	REQUIRE(server.pid() > 0);
	wait_for_server(port);

	Elder elder = first_man(port);
	SessionStream& adam = *elder.session.stream;
	GrpcSession boris = open_session(port);
	REQUIRE(boris.stream);
	const std::string boris_name = born_named(elder, *boris.stream, SenderToken);

	// Turning, whatever else is told on the way.
	const auto turn_to = [](SessionStream& stream, const std::string& host, const std::string& room) {
		send_turn_to_abode(stream, host, room);
		read_until(stream, [](const auto& e) { return e.has_turned(); });
		read_until(stream, [](const auto& e) { return e.has_history_end(); });
	};

	// Adam lets Boris in at his gates and makes him a friend: Boris may look at Adam's word.
	turn_to(*boris.stream, {}, "Врата");
	turn_to(*boris.stream, elder.name, "Врата");
	send_knock(*boris.stream);
	turn_to(adam, {}, "Врата");
	send_admit(adam, boris_name);
	read_until(adam, [](const auto& e) { return e.has_protocol_notice(); });
	turn_to(adam, {}, "Кормчая");
	send_regard(adam, boris_name, will::v1::FRIEND);
	read_until(adam, [](const auto& e) { return e.has_protocol_notice(); });
	turn_to(*boris.stream, elder.name, "Слово");
	turn_to(adam, {}, "Слово");

	// Boris looks at Adam's word and reads no more; Adam writes much, and each word of
	// his is acknowledged at once, however full Boris's way is.
	// He reads his own answers as he goes, as a client does.
	constexpr int Words = 3000;
	constexpr int Batch = 100;
	const std::string body(4000, 'x');
	int acknowledged = 0;
	will::v1::ServerEvent event;
	for (int sent = 0; sent < Words; sent += Batch) {
		for (int i = 0; i < Batch; ++i)
			send_chat(adam, body.c_str());
		while (acknowledged < sent + Batch && adam.Read(&event)) {
			if (event.has_receipt_ack())
				++acknowledged;
		}
	}
	CHECK(acknowledged == Words);

	boris.context->TryCancel();
	elder.session.context->TryCancel();
	server.stop();
}


TEST_CASE("one does not write at the gates nor at the helm, and the session lives on")
{
	const std::uint16_t port = static_cast<std::uint16_t>(pick_port() + 8);
	const std::string directory = "/tmp/will-no-writing-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

	RunningServer server{start_server(g_server_exe, port, directory)};
	REQUIRE(server.pid() > 0);
	wait_for_server(port);

	Elder elder = first_man(port);
	SessionStream& adam = *elder.session.stream;

	for (const char* room : {"Врата", "Кормчая"}) {
		stand_in(adam, {}, room);
		send_chat(adam, "not here");
		will::v1::ServerEvent told;
		REQUIRE(adam.Read(&told));
		// The world refuses it itself: one writes only standing in a cell.
		CHECK(told.protocol_notice().message() == "one does this only standing in a cell");
		CHECK(told.protocol_notice().code() == will::v1::NOTICE_STAND_IN_CELL);
	}

	// He is still there: he turns to his word and writes in it.
	stand_in(adam, {}, "Слово");
	send_chat(adam, "here");
	will::v1::ServerEvent receipt;
	REQUIRE(adam.Read(&receipt));
	CHECK(receipt.receipt_ack().word().body() == "here");

	elder.session.context->TryCancel();
	server.stop();
}


TEST_CASE("the server speaks with TLS beside the open port, and only with TLS once the open port is closed")
{
	const std::uint16_t port = static_cast<std::uint16_t>(pick_port() + 6);
	const std::uint16_t tls_port = static_cast<std::uint16_t>(pick_port() + 7);
	const std::string directory = "/tmp/will-tls-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};
	write_file(directory + "/server.crt", TestCert);
	write_file(directory + "/server.key", TestKey);
	const std::vector<std::string> tls_args{"--port", std::to_string(port), "--db-path", directory,
											"--tls-cert", directory + "/server.crt",
											"--tls-key", directory + "/server.key",
											"--tls-port", std::to_string(tls_port)};
	{
		RunningServer server{start_server_with(g_server_exe, tls_args)};
		REQUIRE(server.pid() > 0);
		wait_for_server(port);
		wait_for_server(tls_port);

		// Over TLS, trusting the server's certificate, one comes in as ever.
		GrpcSession secure = open_tls_session(tls_port, TestCert);
		REQUIRE(secure.stream);
		CHECK_FALSE(bind_named(*secure.stream, ElderToken).empty());

		// The open port still serves the old clients.
		GrpcSession open = open_session(port);
		REQUIRE(open.stream);
		{
			will::v1::ClientEvent bind;
			bind.mutable_bind_token()->set_token(SenderToken);
			REQUIRE(open.stream->Write(bind));
		}
		CHECK(read_event(*open.stream).auth_ok().unborn());

		// In the open the TLS port is not understood.
		GrpcSession wrong = open_session(tls_port);
		REQUIRE(wrong.stream);
		{
			will::v1::ClientEvent bind;
			bind.mutable_bind_token()->set_token(ViewerToken);
			wrong.stream->Write(bind);
		}
		will::v1::ServerEvent nothing;
		CHECK_FALSE(wrong.stream->Read(&nothing));

		wrong.context->TryCancel();
		open.context->TryCancel();
		secure.context->TryCancel();
		server.stop();
	}

	// With the open port closed, only TLS is heard.
	std::vector<std::string> tls_only = tls_args;
	tls_only.push_back("--no-plain");
	RunningServer server{start_server_with(g_server_exe, tls_only)};
	REQUIRE(server.pid() > 0);
	wait_for_server(tls_port);
	const int fd = connect_tcp("127.0.0.1", port);
	CHECK(fd < 0);
	if (fd >= 0)
		::close(fd);
	GrpcSession secure = open_tls_session(tls_port, TestCert);
	REQUIRE(secure.stream);
	CHECK_FALSE(bind_named(*secure.stream, ElderToken).empty());
	secure.context->TryCancel();
	server.stop();
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
