module;

#include "proto/messenger.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

module will.server.grpcserver;

import will.server.connectionaccountstore;
import will.server.protocoladapter;
import will.server.sessionregistry;


namespace will {


namespace {


void enqueue_event(void* registry, const std::uint64_t session_id, const v1::ServerEvent& event)
{
    static_cast<SessionRegistry*>(registry)->enqueue_event(session_id, event);
}


void close_session(void* registry, const std::uint64_t session_id)
{
    static_cast<SessionRegistry*>(registry)->close_session(session_id);
}


std::string_view peer_address(void* registry, const std::uint64_t session_id)
{
    return static_cast<SessionRegistry*>(registry)->peer_address(session_id);
}


void broadcast_chat(void* registry, const std::uint64_t except_session, const std::string_view name,
                    const std::string_view body)
{
    v1::ServerEvent event;
    auto* chat_message = event.mutable_chat();
    chat_message->set_name(std::string(name));
    chat_message->set_body(std::string(body));
    static_cast<SessionRegistry*>(registry)->broadcast_except(except_session, event);
}


std::atomic<GrpcMessengerServer*> g_server_for_signal{nullptr};


void handle_signal(int)
{
    if (GrpcMessengerServer* server = g_server_for_signal.load())
        server->request_stop();
}


class MessengerService final : public v1::Messenger::Service {
public:
    MessengerService(ProtocolAdapter& adapter, SessionRegistry& registry, ConnectionAccountStore& account_store,
                     std::size_t max_connections)
        : adapter_(adapter)
        , registry_(registry)
        , account_store_(account_store)
        , max_connections_(max_connections)
    {}

    grpc::Status Session(grpc::ServerContext* context,
                         grpc::ServerReaderWriter<v1::ServerEvent, v1::ClientEvent>* stream) override
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

private:
    ProtocolAdapter& adapter_;
    SessionRegistry& registry_;
    ConnectionAccountStore& account_store_;
    std::size_t max_connections_;
};


} // namespace


struct GrpcMessengerServer::Impl {
    Impl(ServerConfig config, void* messages, void* users)
        : config_(std::move(config))
        , protocol_adapter_(messages, users, &registry_, &account_store_, &enqueue_event, &close_session,
                            &peer_address, &broadcast_chat)
        , service_(protocol_adapter_, registry_, account_store_, config_.max_connections)
    {}

    ServerConfig config_;
    ConnectionAccountStore account_store_;
    SessionRegistry registry_;
    ProtocolAdapter protocol_adapter_;
    MessengerService service_;
    std::unique_ptr<grpc::Server> server_;
    std::atomic<bool> stopping_{false};
};


GrpcMessengerServer::GrpcMessengerServer(ServerConfig config, void* messages, void* users)
    : impl_(std::make_unique<Impl>(std::move(config), messages, users))
{}


GrpcMessengerServer::~GrpcMessengerServer() = default;


void GrpcMessengerServer::run()
{
    const std::string address = "0.0.0.0:" + std::to_string(impl_->config_.listen_port);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&impl_->service_);
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, impl_->config_.keepalive_interval_seconds * 1000);
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, impl_->config_.keepalive_timeout_seconds * 1000);
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
    builder.AddChannelArgument(GRPC_ARG_HTTP2_MIN_RECV_PING_INTERVAL_WITHOUT_DATA_MS, 5000);

    impl_->server_ = builder.BuildAndStart();
    if (!impl_->server_)
        throw std::runtime_error("Failed to start gRPC server on " + address);

    g_server_for_signal.store(this);
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    // Shutdown must not run in the signal handler (not async-signal-safe; deadlocks in gRPC/absl).
    std::thread shutdown_thread([this] {
        while (!impl_->stopping_.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

        std::cout << "Shutdown signal received\n";
        if (impl_->server_)
            impl_->server_->Shutdown();
    });

    impl_->server_->Wait();

    impl_->stopping_.store(true);
    if (shutdown_thread.joinable())
        shutdown_thread.join();

    g_server_for_signal.store(nullptr);
}


void GrpcMessengerServer::request_stop()
{
    impl_->stopping_.store(true);
}


} // namespace will
