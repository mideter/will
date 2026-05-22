#include "sessionregistry.h"

#include <iostream>
#include <utility>

#include "session.h"
#include "willmessage.h"


namespace will {


void SessionRegistry::accept_session(asio::io_context& ioc, asio::ip::tcp::socket socket,
                                     ClientAddress address, std::size_t max_outbound_queue_bytes)
{
    auto session = std::shared_ptr<Session>(
        new Session(ioc, std::move(socket), std::move(address), *this, max_outbound_queue_bytes));

    {
        std::lock_guard lock(mutex_);
        std::cout << "Client " << session->address() << " connected" << std::endl;
        sessions_.emplace(session->id(), session);
    }

    session->begin();
}


void SessionRegistry::close_session(std::uint64_t session_id)
{
    std::shared_ptr<Session> session;

    {
        std::lock_guard lock(mutex_);
        const auto it = sessions_.find(session_id);

        if (it == sessions_.end())
            return;

        session = it->second;
        sessions_.erase(it);
    }

    std::cout << "Client " << session->address() << " disconnected" << std::endl;
    session->shutdown();
}


void SessionRegistry::shutdown_all()
{
    std::vector<std::shared_ptr<Session>> sessions;

    {
        std::lock_guard lock(mutex_);
        sessions.reserve(sessions_.size());

        for (const auto& [id, session] : sessions_)
            sessions.push_back(session);

        sessions_.clear();
    }

    for (const std::shared_ptr<Session>& session : sessions) {
        std::cout << "Client " << session->address() << " disconnected" << std::endl;
        session->shutdown();
    }
}


std::size_t SessionRegistry::count() const noexcept
{
    std::lock_guard lock(mutex_);
    return sessions_.size();
}


void SessionRegistry::broadcast_except(const Session& sender, const std::vector<char>& payload)
{
    std::cout << "Broadcast from " << sender.address() << ": "
              << WillMessage::format_payload_for_log(payload) << std::endl;

    std::vector<std::shared_ptr<Session>> peers;

    {
        std::lock_guard lock(mutex_);
        peers.reserve(sessions_.size());

        for (const auto& entry : sessions_) {
            if (*entry.second != sender)
                peers.push_back(entry.second);
        }
    }

    for (const std::shared_ptr<Session>& peer : peers)
        peer->enqueue_payload_broadcast(payload);
}


bool SessionRegistry::at_capacity(std::size_t max_connections) const noexcept
{
    std::lock_guard lock(mutex_);
    return sessions_.size() >= max_connections;
}


} // namespace will
