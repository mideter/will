#pragma once

#include <asio.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>


namespace will::domain {


struct ParticipantId;


} // namespace will::domain


namespace will {


class WillProtocolAdapter;
class TcpConnection;


/**
 * Thread-safe registry of connected {@link TcpConnection} instances.
 * Owns connection creation and teardown.
 */
class TcpConnectionRegistry {
public:
    TcpConnectionRegistry() = default;

    TcpConnectionRegistry(const TcpConnectionRegistry&) = delete;
    TcpConnectionRegistry& operator=(const TcpConnectionRegistry&) = delete;

    void accept_connection(asio::io_context& ioc, asio::ip::tcp::socket socket,
                           asio::ip::tcp::endpoint peer_endpoint,
                           WillProtocolAdapter& protocol_adapter,
                           std::size_t max_outbound_queue_bytes);

    void close_connection(std::uint64_t connection_id);

    void close_all_connections();

    std::size_t count() const noexcept;

    /** Logs and fan-out of {@code payload} to every connection except {@code except_participant}. */
    void broadcast_except_participant(domain::ParticipantId except_participant,
                                      const std::vector<char>& payload);

    bool at_capacity(std::size_t max_connections) const noexcept;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::uint64_t, std::shared_ptr<TcpConnection>> connections_;
};


} // namespace will
