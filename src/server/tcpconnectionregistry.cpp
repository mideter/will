#include "tcpconnectionregistry.h"

#include <iostream>
#include <utility>

#include "tcpconnection.h"
#include "willprotocol.h"
#include "willprotocoladapter.h"

#include "entities/participant_id.h"


namespace will {


void TcpConnectionRegistry::accept_connection(asio::io_context& ioc, asio::ip::tcp::socket socket,
                                              asio::ip::tcp::endpoint peer_endpoint,
                                              WillProtocolAdapter& protocol_adapter,
                                              const std::size_t max_outbound_queue_bytes)
{
    TcpConnectionHandlers handlers{
        [&protocol_adapter, this](const std::uint64_t connection_id, std::vector<char> payload) {
            std::shared_ptr<TcpConnection> connection;

            {
                std::lock_guard lock(mutex_);
                const auto it = connections_.find(connection_id);

                if (it == connections_.end())
                    return;
                
                connection = it->second;
            }

            protocol_adapter.on_client_frame(*connection, payload);
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

    std::cout << "Client " << connection->peer_label() << " disconnected" << std::endl;
    connection->shutdown();
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


void TcpConnectionRegistry::broadcast_except_participant(const domain::ParticipantId except_participant,
                                                         const std::vector<char>& payload)
{
    std::vector<std::shared_ptr<TcpConnection>> peers;
    std::string sender_label;

    {
        std::lock_guard lock(mutex_);
        peers.reserve(connections_.size());

        for (const auto& [id, connection] : connections_) {
            if (domain::ParticipantId{id} == except_participant) {
                sender_label = connection->peer_label();
                continue;
            }
            peers.push_back(connection);
        }
    }

    if (!sender_label.empty()) {
        std::cout << "Broadcast from " << sender_label << ": "
                  << WillProtocolAdapter::format_payload_for_log(payload) << std::endl;
    }

    for (const std::shared_ptr<TcpConnection>& peer : peers)
        peer->enqueue_frame(TcpFrame::encode(payload));
}


bool TcpConnectionRegistry::at_capacity(const std::size_t max_connections) const noexcept
{
    std::lock_guard lock(mutex_);
    return connections_.size() >= max_connections;
}


} // namespace will
