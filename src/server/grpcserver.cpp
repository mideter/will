#include "grpcserver.h"

#include <chrono>
#include <csignal>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>


namespace will {


namespace {


std::atomic<GrpcMessengerServer*> g_server_for_signal{nullptr};


void handle_signal(int)
{
    if (GrpcMessengerServer* server = g_server_for_signal.load())
        server->request_stop();
}


} // namespace


MessengerService::MessengerService(ProtocolAdapter& adapter, SessionRegistry& registry,
                                   ConnectionAccountStore& account_store, const std::size_t max_connections)
    : adapter_(adapter)
    , registry_(registry)
    , account_store_(account_store)
    , max_connections_(max_connections)
{}


grpc::Status MessengerService::Session(grpc::ServerContext* context,
                                       grpc::ServerReaderWriter<v1::ServerEvent, v1::ClientEvent>* stream)
{
    if (registry_.at_capacity(max_connections_)) {
        std::cerr << "Max connections (" << max_connections_ << ") reached, rejecting peer\n";
        return grpc::Status(grpc::StatusCode::RESOURCE_EXHAUSTED, "max clients reached");
    }

    const auto session = registry_.register_session(context, stream);
    const std::uint64_t session_id = session->id();

    v1::ClientEvent event;
    while (!session->closed() && stream->Read(&event)) {
        adapter_.on_client_event(session_id, event);
        if (session->closed())
            break;
    }

    account_store_.remove(session_id);
    registry_.unregister_session(session_id);
    return grpc::Status::OK;
}


GrpcMessengerServer::GrpcMessengerServer(ServerConfig config, domain::MessengerPersistence persistence)
    : config_(std::move(config))
    , protocol_adapter_(persistence, registry_, account_store_)
    , service_(protocol_adapter_, registry_, account_store_, config_.max_connections)
{}


void GrpcMessengerServer::run()
{
    const std::string address = "0.0.0.0:" + std::to_string(config_.listen_port);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, config_.keepalive_interval_seconds * 1000);
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, config_.keepalive_timeout_seconds * 1000);
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
    builder.AddChannelArgument(GRPC_ARG_HTTP2_MIN_RECV_PING_INTERVAL_WITHOUT_DATA_MS, 5000);

    server_ = builder.BuildAndStart();
    if (!server_)
        throw std::runtime_error("Failed to start gRPC server on " + address);

    g_server_for_signal.store(this);
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    // Shutdown must not run in the signal handler (not async-signal-safe; deadlocks in gRPC/absl).
    std::thread shutdown_thread([this] {
        while (!stopping_.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

        std::cout << "Shutdown signal received\n";
        if (server_)
            server_->Shutdown();
    });

    server_->Wait();

    stopping_.store(true);
    if (shutdown_thread.joinable())
        shutdown_thread.join();

    g_server_for_signal.store(nullptr);
}


void GrpcMessengerServer::request_stop()
{
    stopping_.store(true);
}


} // namespace will
