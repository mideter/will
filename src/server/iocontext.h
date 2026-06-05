#pragma once

#include <asio.hpp>

#include <cstdint>
#include <functional>


namespace will {


/** io_context with bound TCP acceptor and POSIX shutdown signals. */
class IoContext {
public:
    using SignalHandler = std::function<void(const asio::error_code&, int)>;

    IoContext(std::uint16_t listen_port, int listen_backlog, SignalHandler on_signal);

    asio::io_context& ioc() noexcept;
    asio::ip::tcp::acceptor& acceptor() noexcept;

    void close_acceptor();
    void stop() noexcept;

private:
    void open_acceptor(std::uint16_t listen_port, int listen_backlog);
    void setup_signals(SignalHandler on_signal);

    asio::io_context ioc_;
    asio::ip::tcp::acceptor acceptor_;
    asio::signal_set signals_;
};


} // namespace will
