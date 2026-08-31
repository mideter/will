#include "infra/transport/messenger.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>


namespace {


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


void bind_device_token(SessionStream& stream, const char* device_token)
{
    will::v1::ClientEvent event;
    event.mutable_bind_token()->set_token(device_token);
    assert(stream.Write(event));

    will::v1::ServerEvent response;
    assert(stream.Read(&response));
    assert(response.has_auth_ok());
}


void drain_receipt_ack(SessionStream& stream)
{
    will::v1::ServerEvent event;
    assert(stream.Read(&event));
    assert(event.has_receipt_ack());
}


void send_chat(SessionStream& stream, const char* body)
{
    will::v1::ClientEvent event;
    event.mutable_chat()->set_body(body);
    assert(stream.Write(event));
}


void send_history_request(SessionStream& stream, std::uint32_t limit)
{
    will::v1::ClientEvent event;
    event.mutable_history_request()->set_limit(limit);
    assert(stream.Write(event));
}


constexpr const char* SenderToken = "11111111111111111111111111111111";
constexpr const char* ViewerToken = "22222222222222222222222222222222";


std::uint16_t pick_port()
{
    return static_cast<std::uint16_t>(20000 + (getpid() % 10000));
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
    assert(false && "server did not start");
}


void print_usage(const char* program)
{
    std::cerr << "Usage: " << program << " <path-to-will-server>\n";
}


} // namespace


int main(int argc, char* argv[])
{
    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    const char* const server_exe = argv[1];
    const std::uint16_t port = pick_port();
    const std::string db_path = "/tmp/will-history-test-" + std::to_string(getpid()) + ".db";

    ::unlink(db_path.c_str());

    const pid_t server_pid = start_server(server_exe, port, db_path);
    assert(server_pid > 0);
    wait_for_server(port);

    GrpcSession sender = open_session(port);
    assert(sender.stream);
    bind_device_token(*sender.stream, SenderToken);
    send_chat(*sender.stream, "hello-from-sender");
    drain_receipt_ack(*sender.stream);

    GrpcSession viewer = open_session(port);
    assert(viewer.stream);
    bind_device_token(*viewer.stream, ViewerToken);
    send_history_request(*viewer.stream, 10);

    will::v1::ServerEvent first_item;
    assert(viewer.stream->Read(&first_item));
    assert(first_item.has_history_item());
    assert(first_item.history_item().body() == "hello-from-sender");
    assert(!first_item.history_item().is_mine());
    assert(!first_item.history_item().name().empty());
    assert(first_item.history_item().name().size() == 8);

    will::v1::ServerEvent end;
    assert(viewer.stream->Read(&end));
    assert(end.has_history_end());

    send_chat(*sender.stream, "hello-again");
    drain_receipt_ack(*sender.stream);
    send_history_request(*sender.stream, 10);

    will::v1::ServerEvent own_item;
    assert(sender.stream->Read(&own_item));
    assert(own_item.has_history_item());
    assert(own_item.history_item().body() == "hello-from-sender");
    assert(own_item.history_item().is_mine());
    assert(!own_item.history_item().name().empty());

    will::v1::ServerEvent own_second;
    assert(sender.stream->Read(&own_second));
    assert(own_second.has_history_item());
    assert(own_second.history_item().body() == "hello-again");
    assert(own_second.history_item().is_mine());
    assert(own_second.history_item().name() == own_item.history_item().name());

    will::v1::ServerEvent sender_end;
    assert(sender.stream->Read(&sender_end));
    assert(sender_end.has_history_end());

    sender.context->TryCancel();
    viewer.context->TryCancel();
    stop_server(server_pid);
    ::unlink(db_path.c_str());

    std::cout << "history integration test passed\n";
    return EXIT_SUCCESS;
}
