#include "tcpconnectionregistry.h"

#include <iostream>
#include <utility>

#include "tcpconnection.h"


namespace will {


TcpConnectionRegistry::TcpConnectionRegistry(ConnectionAccountStore& account_store)
    : account_store_(account_store)
{}


void TcpConnectionRegistry::set_frame_handler(
    std::function<void(std::uint64_t, const std::vector<char>&)> handler)
{
    frame_handler_ = std::move(handler);
}


void TcpConnectionRegistry::accept_connection(asio::io_context& ioc, asio::ip::tcp::socket socket,
                                              asio::ip::tcp::endpoint peer_endpoint,
                                              const std::size_t max_outbound_queue_bytes)
{
    TcpConnectionHandlers handlers{
        [this](const std::uint64_t connection_id, std::vector<char> payload) {
            if (frame_handler_)
                frame_handler_(connection_id, payload);
        },
        [this](const std::uint64_t connection_id) { close_connection(connection_id); },
    };

    auto connection = std::shared_ptr<TcpConnection>(
        new TcpConnection(ioc, std::move(socket), std::move(peer_endpoint), std::move(handlers)));

    {
        std::lock_guard lock(mutex_);
        std::cout << "Client " << connection->peer_label() << " connected" << std::endl;
        connections_.emplace(connection->id(), connection);
    }

    connection->begin(max_outbound_queue_bytes);
}


void TcpConnectionRegistry::close_connection(const std::uint64_t connection_id)
{
    std::shared_ptr<TcpConnection> connection;

    {
        std::lock_guard lock(mutex_);
        const auto it = connections_.find(connection_id);

        if (it == connections_.end())
            return;

        connection = it->second;
        connections_.erase(it);
    }

    account_store_.remove(connection_id);

    std::cout << "Client " << connection->peer_label() << " disconnected" << std::endl;
    connection->shutdown();
}


void TcpConnectionRegistry::enqueue_wire_frame(const std::uint64_t connection_id,
                                               std::vector<char> wire_bytes)
{
    std::shared_ptr<TcpConnection> connection;

    {
        std::lock_guard lock(mutex_);
        const auto it = connections_.find(connection_id);

        if (it == connections_.end())
            return;

        connection = it->second;
    }

    connection->enqueue_frame(std::move(wire_bytes));
}


void TcpConnectionRegistry::broadcast_wire_except(const std::uint64_t except_connection_id,
                                                  const std::vector<char>& wire_bytes)
{
    std::vector<std::shared_ptr<TcpConnection>> peers;

    {
        std::lock_guard lock(mutex_);
        peers.reserve(connections_.size());

        for (const auto& [id, connection] : connections_) {
            if (id == except_connection_id)
                continue;

            peers.push_back(connection);
        }
    }

    for (const std::shared_ptr<TcpConnection>& peer : peers)
        peer->enqueue_frame(wire_bytes);
}


std::string_view TcpConnectionRegistry::peer_label(const std::uint64_t connection_id) const
{
    std::lock_guard lock(mutex_);
    const auto it = connections_.find(connection_id);

    if (it == connections_.end())
        return {};

    return it->second->peer_label();
}


void TcpConnectionRegistry::close_all_connections()
{
    std::vector<std::uint64_t> connection_ids;

    {
        std::lock_guard lock(mutex_);
        connection_ids.reserve(connections_.size());

        for (const auto& [id, connection] : connections_)
            connection_ids.push_back(id);
    }

    for (const std::uint64_t connection_id : connection_ids)
        close_connection(connection_id);
}


std::size_t TcpConnectionRegistry::count() const noexcept
{
    std::lock_guard lock(mutex_);
    return connections_.size();
}


bool TcpConnectionRegistry::at_capacity(const std::size_t max_connections) const noexcept
{
    std::lock_guard lock(mutex_);
    return connections_.size() >= max_connections;
}


} // namespace will
