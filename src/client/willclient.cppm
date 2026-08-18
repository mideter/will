module;

#include "proto/messenger.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string_view>
#include <thread>

export module will.client.willclient;

export import will.client.clientconfig;
export import will.client.consoleui;

export namespace will {


class WillClient {
public:
    using ServerEvent = v1::ServerEvent;
    using ClientEvent = v1::ClientEvent;

    WillClient();
    explicit WillClient(ClientConfig config);

    /** Opens a gRPC channel and starts Messenger.Session. */
    void connect();

    /** Sends BindToken and waits for AuthOk. */
    void authenticate_device(std::string_view device_token);

    /** Called from the inbound reader thread. */
    void set_inbound_handler(std::function<void(const ServerEvent&)> handler);
    void set_closed_handler(std::function<void()> handler);

    void send(std::string_view utf8_chat_body) const;

    /** Sends HistoryRequest; returns false when limit is 0. */
    bool requestHistory(std::uint32_t limit) const;

    void shutdown() const;

    const ClientConfig& config() const noexcept;

private:
    v1::ServerEvent wait_for_auth_response();
    void dispatch_inbound(const v1::ServerEvent& event);
    void dispatch_closed();
    void reader_loop();
    bool write_event(const v1::ClientEvent& event) const;

    ClientConfig config_;

    mutable std::shared_ptr<grpc::Channel> channel_;
    mutable std::unique_ptr<v1::Messenger::Stub> stub_;
    mutable std::unique_ptr<grpc::ClientContext> context_;
    using Stream = grpc::ClientReaderWriter<v1::ClientEvent, v1::ServerEvent>;
    mutable std::shared_ptr<Stream> stream_;

    mutable std::mutex write_mutex_;
    mutable std::jthread reader_thread_;

    mutable std::mutex handler_mutex_;
    std::function<void(const v1::ServerEvent&)> inbound_handler_;
    std::function<void()> closed_handler_;

    std::shared_ptr<std::promise<v1::ServerEvent>> pending_auth_;
    mutable bool authenticated_ = false;
    mutable bool shutdown_done_ = false;
    mutable std::atomic<bool> closed_{false};
};


class ChatSession {
public:
    ChatSession(WillClient& client, ConsoleUi& ui);

    void run();

private:
    void loadHistory() const;

    WillClient& client_;
    ConsoleUi& ui_;
};


} // namespace will
