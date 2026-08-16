#include <atomic>
#include <chrono>
#include <cstdlib>
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "clientconfigvalidator.h"
#include "loadclientsconfigparser.h"
#include "proto/messenger.grpc.pb.h"
#include "support/device_token.h"


namespace {


std::string device_token_for_client(const std::size_t client_index)
{
    return std::format("{:032x}", client_index + 1);
}


void bind_device_token(grpc::ClientReaderWriter<will::v1::ClientEvent, will::v1::ServerEvent>& stream,
                      const std::string& device_token)
{
    will::v1::ClientEvent event;
    event.mutable_bind_token()->set_token(device_token);
    if (!stream.Write(event))
        throw std::runtime_error("failed to send BindToken");

    will::v1::ServerEvent response;
    if (!stream.Read(&response))
        throw std::runtime_error("failed to read auth response");

    if (!response.has_auth_ok())
        throw std::runtime_error("device authentication failed");
}


void client_worker(const will::LoadClientsConfig& config, const std::size_t client_index,
                   std::atomic<std::size_t>& connect_failures)
{
    const will::ClientConfig connection = config.connection;
    const std::string target = connection.host + ":" + std::to_string(connection.port);

    try {
        auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
        auto stub = will::v1::Messenger::NewStub(channel);
        grpc::ClientContext context;
        auto stream = stub->Session(&context);
        if (!stream)
            throw std::runtime_error("failed to open Session");

        bind_device_token(*stream, device_token_for_client(client_index));

        for (std::size_t i = 0; i < config.messages_per_client; ++i) {
            will::v1::ClientEvent event;
            event.mutable_chat()->set_body("load-" + std::to_string(i));
            if (!stream->Write(event))
                throw std::runtime_error("failed to send chat");
        }

        if (config.hold_seconds > 0)
            std::this_thread::sleep_for(std::chrono::seconds(config.hold_seconds));

        stream->WritesDone();
        context.TryCancel();
    }
    catch (...) {
        connect_failures.fetch_add(1, std::memory_order_relaxed);
    }
}


} // namespace


int main(int argc, char* argv[])
try {
    const will::LoadClientsConfigParser cli(argc, argv);
    will::LoadClientsConfig config = cli.load_config();
    config.connection = will::ClientConfigValidator::accept(std::move(config.connection));

    std::atomic<std::size_t> failures{0};

    const auto started = std::chrono::steady_clock::now();

    std::vector<std::thread> threads;
    threads.reserve(config.clients);
    for (std::size_t i = 0; i < config.clients; ++i)
        threads.emplace_back(client_worker, std::cref(config), i, std::ref(failures));

    for (std::thread& t : threads)
        t.join();

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() -
                                                              started);

    const std::size_t ok = config.clients - failures.load();
    std::cout << "load_clients: clients=" << config.clients << " ok=" << ok
              << " failures=" << failures.load() << " elapsed_ms=" << elapsed.count() << '\n';

    return failures.load() == 0 ? 0 : 1;
}
catch (const std::exception& e) {
    std::cerr << "load_clients error: " << e.what() << '\n';
    return 1;
}
