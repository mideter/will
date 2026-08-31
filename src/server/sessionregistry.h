#pragma once

#include "infra/transport/messenger.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>


namespace will {


class SessionStream {
public:
    using Stream = grpc::ServerReaderWriter<v1::ServerEvent, v1::ClientEvent>;

    SessionStream(std::uint64_t id, grpc::ServerContext* context, Stream* stream, std::string peer_address);

    std::uint64_t id() const noexcept { return id_; }
    std::string_view peer_address() const noexcept { return peer_address_; }
    bool closed() const noexcept { return closed_.load(); }

    bool write(const v1::ServerEvent& event);
    void request_close();

private:
    const std::uint64_t id_;
    grpc::ServerContext* context_;
    Stream* stream_;
    std::string peer_address_;
    std::mutex write_mutex_;
    std::atomic<bool> closed_{false};
};


/**
 * Thread-safe registry of active gRPC Session streams.
 * Owns session lifecycle for fan-out and takeover.
 */
class SessionRegistry {
public:
    SessionRegistry() = default;

    SessionRegistry(const SessionRegistry&) = delete;
    SessionRegistry& operator=(const SessionRegistry&) = delete;

    std::shared_ptr<SessionStream> register_session(grpc::ServerContext* context, SessionStream::Stream* stream);

    void unregister_session(std::uint64_t session_id);

    void close_session(std::uint64_t session_id);

    void enqueue_event(std::uint64_t session_id, const v1::ServerEvent& event);

    void broadcast_except(std::uint64_t except_session_id, const v1::ServerEvent& event);

    [[nodiscard]] std::string_view peer_address(std::uint64_t session_id) const;

    void close_all_sessions();

    std::size_t count() const noexcept;

    bool at_capacity(std::size_t max_connections) const noexcept;

private:
    static std::string peer_from_context(grpc::ServerContext* context);

    mutable std::mutex mutex_;
    std::unordered_map<std::uint64_t, std::shared_ptr<SessionStream>> sessions_;
    std::atomic<std::uint64_t> next_id_{1};
};


} // namespace will
