#include "ioresources.h"

#include <csignal>


namespace will {


IoResources::IoResources(std::uint16_t listen_port, int listen_backlog, SignalHandler on_signal)
    : acceptor_(ioc_)
    , signals_(ioc_)
{
    open_acceptor(listen_port, listen_backlog);
    setup_signals(std::move(on_signal));
}


asio::io_context& IoResources::ioc() noexcept
{
    return ioc_;
}


asio::ip::tcp::acceptor& IoResources::acceptor() noexcept
{
    return acceptor_;
}


void IoResources::open_acceptor(std::uint16_t listen_port, int listen_backlog)
{
    using asio::ip::tcp;
    const tcp::endpoint endpoint(tcp::v4(), listen_port);

    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen(listen_backlog);
}


void IoResources::setup_signals(SignalHandler on_signal)
{
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
    signals_.async_wait(std::move(on_signal));
}


void IoResources::close_acceptor()
{
    asio::error_code ignored;
    acceptor_.close(ignored);
}


void IoResources::stop() noexcept
{
    ioc_.stop();
}


} // namespace will
