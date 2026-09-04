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


void SessionRegistry::clear_soul_binding(const SessionId session_id)
{
    const auto it = sessions_.find(session_id.value);
    if (it == sessions_.end())
        return;

    if (const auto bound_soul = it->second->soul_id()) {
        const auto rit = session_by_soul_.find(bound_soul->value());
        if (rit != session_by_soul_.end() && rit->second == session_id)
            session_by_soul_.erase(rit);
    }

    it->second->clear_soul_id();
}


void SessionRegistry::unregister_session(const SessionId session_id)
{
    std::lock_guard lock(mutex_);
    clear_soul_binding(session_id);
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
        clear_soul_binding(session_id);
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


void SessionRegistry::broadcast_except_soul(const domain::id::Soul except_soul_id, const v1::ServerEvent& event)
{
    std::vector<std::shared_ptr<Session>> targets;
    {
        std::lock_guard lock(mutex_);
        targets.reserve(sessions_.size());
        for (const auto& [id, session] : sessions_) {
            if (const auto bound = session->soul_id(); bound && *bound == except_soul_id)
                continue;
            targets.push_back(session);
        }
    }

    for (const auto& session : targets) {
        if (!session->write(event))
            close_session(session->id());
    }
}


std::optional<SessionId> SessionRegistry::session_id_for_soul(const domain::id::Soul soul_id) const
{
    std::lock_guard lock(mutex_);
    const auto it = session_by_soul_.find(soul_id.value());
    if (it == session_by_soul_.end())
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


std::optional<domain::id::Soul> SessionRegistry::soul_id(const SessionId session_id) const
{
    std::lock_guard lock(mutex_);
    const auto it = sessions_.find(session_id.value);
    if (it == sessions_.end())
        return std::nullopt;
    return it->second->soul_id();
}


std::optional<SessionId> SessionRegistry::bind_soul(const SessionId session_id, const domain::id::Soul soul_id)
{
    std::lock_guard lock(mutex_);

    const auto it = sessions_.find(session_id.value);
    if (it == sessions_.end())
        return std::nullopt;

    const auto& session = it->second;

    if (const auto existing = session->soul_id()) {
        if (*existing != soul_id) {
            const auto rit = session_by_soul_.find(existing->value());
            if (rit != session_by_soul_.end() && rit->second == session_id)
                session_by_soul_.erase(rit);
        }
    }

    std::optional<SessionId> displaced;
    if (const auto rit = session_by_soul_.find(soul_id.value()); rit != session_by_soul_.end()) {
        if (rit->second != session_id) {
            displaced = rit->second;
            clear_soul_binding(*displaced);
        }
    }

    session->set_soul_id(soul_id);
    session_by_soul_[soul_id.value()] = session_id;
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
        session_by_soul_.clear();
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
