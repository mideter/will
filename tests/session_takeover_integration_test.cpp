#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include "infra/transport/messenger.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
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
	session.stream = session.stub->Session(session.context.get());
	return session;
}


bool bind_device_token(SessionStream& stream, const char* device_token)
{
	will::v1::ClientEvent event;
	event.mutable_bind_token()->set_token(device_token);
	if (!stream.Write(event))
		return false;

	will::v1::ServerEvent response;
	if (!stream.Read(&response))
		return false;
	return response.has_auth_ok();
}


bool drain_receipt_ack(SessionStream& stream)
{
	will::v1::ServerEvent event;
	if (!stream.Read(&event))
		return false;
	return event.has_receipt_ack();
}


/** Wait until stream Read fails (peer cancelled/closed), or timeout. */
bool wait_for_stream_end(GrpcSession& session, std::chrono::milliseconds timeout)
{
	std::atomic<bool> done{false};
	std::atomic<bool> ended{false};

	std::thread reader([&] {
		will::v1::ServerEvent event;
		ended.store(!session.stream->Read(&event));
		done.store(true);
	});

	const auto deadline = std::chrono::steady_clock::now() + timeout;
	while (!done.load() && std::chrono::steady_clock::now() < deadline)
		std::this_thread::sleep_for(std::chrono::milliseconds(20));

	if (!done.load()) {
		session.context->TryCancel();
		reader.join();
		return false;
	}

	reader.join();
	return ended.load();
}


std::uint16_t pick_port()
{
	return static_cast<std::uint16_t>(21000 + (getpid() % 10000));
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


bool wait_for_server(std::uint16_t port)
{
	for (int attempt = 0; attempt < 50; ++attempt) {
		const int fd = connect_tcp("127.0.0.1", port);
		if (fd >= 0) {
			::close(fd);
			return true;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	return false;
}


constexpr const char* DeviceToken = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";


} // namespace


TEST_CASE("second session with same device token displaces the first")
{
	const std::uint16_t port = pick_port();
	const std::string db_path = "/tmp/will-session-takeover-test-" + std::to_string(getpid()) + ".db";

	::unlink(db_path.c_str());

	const pid_t server_pid = start_server(g_server_exe, port, db_path);
	REQUIRE(server_pid > 0);
	REQUIRE(wait_for_server(port));

	GrpcSession first = open_session(port);
	REQUIRE(first.stream);
	REQUIRE(bind_device_token(*first.stream, DeviceToken));

	GrpcSession second = open_session(port);
	REQUIRE(second.stream);
	REQUIRE(bind_device_token(*second.stream, DeviceToken));

	REQUIRE(wait_for_stream_end(first, std::chrono::seconds(2)));

	will::v1::ClientEvent chat;
	chat.mutable_chat()->set_body("after-takeover");
	REQUIRE(second.stream->Write(chat));
	REQUIRE(drain_receipt_ack(*second.stream));

	second.context->TryCancel();
	stop_server(server_pid);
	::unlink(db_path.c_str());
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
