#pragma once

#include <asio.hpp>

#include "connectionaccountstore.h"
#include "tcpstreamsocket.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string_view>
#include <unordered_map>
#include <vector>


namespace will {


class TcpConnection;


/**
 * Thread-safe registry of connected {@link TcpConnection} instances.
 * Owns connection creation and teardown.
 */
class TcpConnectionRegistry {
public:
    explicit TcpConnectionRegistry(ConnectionAccountStore& account_store);

    TcpConnectionRegistry(const TcpConnectionRegistry&) = delete;
    TcpConnectionRegistry& operator=(const TcpConnectionRegistry&) = delete;

    void set_payload_handler(std::function<void(std::uint64_t, const std::vector<char>&)> handler);

    void accept_connection(asio::io_context& ioc, TcpStreamSocket socket,
                           asio::ip::tcp::endpoint peer_endpoint,
                           std::size_t max_outbound_queue_bytes);

    void close_connection(std::uint64_t connection_id);

    void enqueue_wire_frame(std::uint64_t connection_id, std::vector<char> wire_bytes);

    void broadcast_wire_except(std::uint64_t except_connection_id, const std::vector<char>& wire_bytes);

    [[nodiscard]] std::string_view peer_label(std::uint64_t connection_id) const;

    void close_all_connections();

    std::size_t count() const noexcept;

    bool at_capacity(std::size_t max_connections) const noexcept;

private:
    ConnectionAccountStore& account_store_;
    std::function<void(std::uint64_t, const std::vector<char>&)> payload_handler_;

    mutable std::mutex mutex_;
    std::unordered_map<std::uint64_t, std::shared_ptr<TcpConnection>> connections_;
};


} // namespace will
