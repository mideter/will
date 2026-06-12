#pragma once

#include "tcpframedchannel.h"
#include "tcpstreamsocket.h"

#include <asio.hpp>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>


namespace will {


struct TcpConnectionHandlers {
    std::function<void(std::uint64_t connection_id, std::vector<char> payload)> on_payload;
    std::function<void(std::uint64_t connection_id)> on_closed;
};


class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    using Strand = asio::strand<asio::io_context::executor_type>;

    std::uint64_t id() const noexcept { return id_; }
    std::string_view peer_label() const noexcept { return peer_label_; }

    void enqueue_wire_frame(std::vector<char> wire_bytes);

private:
    TcpConnection(asio::io_context& ioc, TcpStreamSocket socket, asio::ip::tcp::endpoint peer_endpoint,
                  TcpConnectionHandlers handlers);

    void begin();
    void shutdown();

    void handle_payload(std::vector<char> payload);
    void request_close();

    const std::uint64_t id_;
    TcpConnectionHandlers handlers_;

    TcpStreamSocket socket_;
    Strand strand_;

    std::string peer_label_;

    std::shared_ptr<TcpFramedChannel> channel_;

    bool closed_ = false;
    bool shutdown_done_ = false;

    static std::atomic<std::uint64_t> next_id_;

    friend class TcpConnectionRegistry;
};


} // namespace will
