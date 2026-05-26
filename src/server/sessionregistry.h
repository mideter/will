#pragma once

#include <asio.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "hostaddress.h"
#include "messagestore.h"


namespace will {


class Session;


/**
 * Thread-safe registry of connected {@link Session} instances.
 * Owns session creation and teardown.
 */
class SessionRegistry {
public:
    explicit SessionRegistry(MessageStore& message_store);

    SessionRegistry(const SessionRegistry&) = delete;
    SessionRegistry& operator=(const SessionRegistry&) = delete;
    SessionRegistry(SessionRegistry&&) = delete;
    SessionRegistry& operator=(SessionRegistry&&) = delete;

    void accept_session(asio::io_context& ioc, asio::ip::tcp::socket socket, HostAddress address,
                        std::size_t max_outbound_queue_bytes);

    void close_session(std::uint64_t session_id);

    void close_all_sessions();

    std::size_t count() const noexcept;

    /** Logs and fan-out of {@code payload} to every session except {@code sender}. */
    void broadcast_except(const Session& sender, const std::vector<char>& payload);

    bool at_capacity(std::size_t max_connections) const noexcept;

private:
    MessageStore& message_store_;
    mutable std::mutex mutex_;
    std::unordered_map<std::uint64_t, std::shared_ptr<Session>> sessions_;
};


} // namespace will
