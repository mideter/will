#pragma once

#include <asio.hpp>

#include "entities/account.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>


namespace will {


class WillProtocolAdapter;
class TcpConnectionRegistry;
class TcpFrameReader;
class TcpFrameWriter;


class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    using TcpSocket = asio::ip::tcp::socket;
    using Strand = asio::strand<asio::io_context::executor_type>;

    std::uint64_t id() const noexcept { return id_; }
    std::string_view peer_label() const noexcept { return peer_label_; }
    std::string_view peer_ip() const noexcept { return peer_ip_; }

    [[nodiscard]] bool has_account() const noexcept { return account_.has_value(); }
    [[nodiscard]] const std::optional<domain::Account>& account() const noexcept { return account_; }
    void set_account(domain::Account account) { account_ = std::move(account); }

    bool operator==(const TcpConnection& other) const noexcept { return id_ == other.id_; }
    bool operator!=(const TcpConnection& other) const noexcept { return !(*this == other); }

    void send_will_payload(const std::vector<char>& payload);
    void fail_protocol(std::string_view message);

private:
    TcpConnection(asio::io_context& ioc, TcpSocket socket, asio::ip::tcp::endpoint peer_endpoint,
                  TcpConnectionRegistry& registry, WillProtocolAdapter& protocol_adapter,
                  std::size_t max_outbound_queue_bytes);

    void begin();
    void shutdown();

    void handle_frame(std::vector<char> payload);
    void handle_read_error(std::string_view context, const asio::error_code& ec);
    void handle_write_queue_full();
    void handle_write_error(std::string_view context, const asio::error_code& ec);
    void fail(std::string_view context, const asio::error_code& ec);

    void enqueue_payload_broadcast(const std::vector<char>& payload);

    const std::uint64_t id_;
    TcpConnectionRegistry& registry_;
    WillProtocolAdapter& protocol_adapter_;
    const std::size_t max_outbound_queue_bytes_;

    TcpSocket socket_;
    Strand strand_;
    std::string peer_ip_;
    std::string peer_label_;

    std::shared_ptr<TcpFrameReader> frame_reader_;
    std::shared_ptr<TcpFrameWriter> frame_writer_;
    std::optional<domain::Account> account_;
    bool closed_ = false;

    static std::atomic<std::uint64_t> next_id_;

    friend class TcpConnectionRegistry;
};


} // namespace will
