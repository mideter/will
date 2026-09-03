#include "sessionregistry.h"

#include <utility>


namespace will {


std::string SessionRegistry::peer_from_context(grpc::ServerContext* context)
{
    if (context == nullptr)
        return {};

    return std::string(context->peer());
}


std::shared_ptr<Session> SessionRegistry::register_session(grpc::ServerContext* context, Session::Stream* stream)
{
    const SessionId id{next_id_.fetch_add(1)};
    auto session = std::make_shared<Session>(id, context, stream, peer_from_context(context));

    std::lock_guard lock(mutex_);
    sessions_.emplace(id.value, session);
    return session;
}


std::shared_ptr<Session> SessionRegistry::register_test_session()
{
    const SessionId id{next_id_.fetch_add(1)};
    auto session = std::make_shared<Session>(id, nullptr, nullptr, std::string{});

    std::lock_guard lock(mutex_);
    sessions_.emplace(id.value, session);
    return session;
}


void SessionRegistry::clear_god_binding(const SessionId session_id)
{
    const auto it = sessions_.find(session_id.value);
    if (it == sessions_.end())
        return;

    if (const auto bound_god = it->second->god_id()) {
        const auto rit = session_by_god_.find(bound_god->value());
        if (rit != session_by_god_.end() && rit->second == session_id)
            session_by_god_.erase(rit);
    }

    it->second->clear_god_id();
}


void SessionRegistry::unregister_session(const SessionId session_id)
{
    std::lock_guard lock(mutex_);
    clear_god_binding(session_id);
    sessions_.erase(session_id.value);
}


void SessionRegistry::close_session(const SessionId session_id)
{
    std::shared_ptr<Session> session;
    {
        std::lock_guard lock(mutex_);
        const auto it = sessions_.find(session_id.value);
        if (it == sessions_.end())
            return;
        session = it->second;
        clear_god_binding(session_id);
        sessions_.erase(it);
    }

    session->request_close();
}


void SessionRegistry::enqueue_event(const SessionId session_id, const v1::ServerEvent& event)
{
    std::shared_ptr<Session> session;
    {
        std::lock_guard lock(mutex_);
        const auto it = sessions_.find(session_id.value);
        if (it == sessions_.end())
            return;
        session = it->second;
    }

    if (!session->write(event))
        close_session(session_id);
}


void SessionRegistry::broadcast_except(const SessionId except_session_id, const v1::ServerEvent& event)
{
    std::vector<std::shared_ptr<Session>> targets;
    {
        std::lock_guard lock(mutex_);
        targets.reserve(sessions_.size());
        for (const auto& [id, session] : sessions_) {
            if (id != except_session_id.value)
                targets.push_back(session);
        }
    }

    for (const auto& session : targets) {
        if (!session->write(event))
            close_session(session->id());
    }
}


void SessionRegistry::broadcast_except_god(const domain::id::God except_god_id, const v1::ServerEvent& event)
{
    std::vector<std::shared_ptr<Session>> targets;
    {
        std::lock_guard lock(mutex_);
        targets.reserve(sessions_.size());
        for (const auto& [id, session] : sessions_) {
            if (const auto bound = session->god_id(); bound && *bound == except_god_id)
                continue;
            targets.push_back(session);
        }
    }

    for (const auto& session : targets) {
        if (!session->write(event))
            close_session(session->id());
    }
}


std::optional<SessionId> SessionRegistry::session_id_for_god(const domain::id::God god_id) const
{
    std::lock_guard lock(mutex_);
    const auto it = session_by_god_.find(god_id.value());
    if (it == session_by_god_.end())
        return std::nullopt;
    return it->second;
}


std::string_view SessionRegistry::peer_address(const SessionId session_id) const
{
    std::lock_guard lock(mutex_);
    const auto it = sessions_.find(session_id.value);
    if (it == sessions_.end())
        return {};
    return it->second->peer_address();
}


bool SessionRegistry::is_authenticated(const SessionId session_id) const
{
    std::lock_guard lock(mutex_);
    const auto it = sessions_.find(session_id.value);
    if (it == sessions_.end())
        return false;
    return it->second->is_authenticated();
}


std::optional<domain::id::God> SessionRegistry::god_id(const SessionId session_id) const
{
    std::lock_guard lock(mutex_);
    const auto it = sessions_.find(session_id.value);
    if (it == sessions_.end())
        return std::nullopt;
    return it->second->god_id();
}


std::optional<SessionId> SessionRegistry::bind_god(const SessionId session_id, const domain::id::God god_id)
{
    std::lock_guard lock(mutex_);

    const auto it = sessions_.find(session_id.value);
    if (it == sessions_.end())
        return std::nullopt;

    const auto& session = it->second;

    if (const auto existing = session->god_id()) {
        if (*existing != god_id) {
            const auto rit = session_by_god_.find(existing->value());
            if (rit != session_by_god_.end() && rit->second == session_id)
                session_by_god_.erase(rit);
        }
    }

    std::optional<SessionId> displaced;
    if (const auto rit = session_by_god_.find(god_id.value()); rit != session_by_god_.end()) {
        if (rit->second != session_id) {
            displaced = rit->second;
            clear_god_binding(*displaced);
        }
    }

    session->set_god_id(god_id);
    session_by_god_[god_id.value()] = session_id;
    return displaced;
}


void SessionRegistry::close_all_sessions()
{
    std::vector<std::shared_ptr<Session>> sessions;
    {
        std::lock_guard lock(mutex_);
        sessions.reserve(sessions_.size());
        for (auto& [id, session] : sessions_)
            sessions.push_back(session);
        sessions_.clear();
        session_by_god_.clear();
    }

    for (const auto& session : sessions)
        session->request_close();
}


std::size_t SessionRegistry::count() const noexcept
{
    std::lock_guard lock(mutex_);
    return sessions_.size();
}


bool SessionRegistry::at_capacity(const std::size_t max_connections) const noexcept
{
    return count() >= max_connections;
}


} // namespace will
