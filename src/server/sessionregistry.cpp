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


void SessionRegistry::clear_user_binding(const SessionId session_id)
{
    const auto it = sessions_.find(session_id.value);
    if (it == sessions_.end())
        return;

    if (const auto bound_user = it->second->user_id()) {
        const auto rit = session_by_user_.find(bound_user->value());
        if (rit != session_by_user_.end() && rit->second == session_id)
            session_by_user_.erase(rit);
    }

    it->second->clear_user_id();
}


void SessionRegistry::unregister_session(const SessionId session_id)
{
    std::lock_guard lock(mutex_);
    clear_user_binding(session_id);
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
        clear_user_binding(session_id);
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


std::optional<domain::UserId> SessionRegistry::user_id(const SessionId session_id) const
{
    std::lock_guard lock(mutex_);
    const auto it = sessions_.find(session_id.value);
    if (it == sessions_.end())
        return std::nullopt;
    return it->second->user_id();
}


std::optional<SessionId> SessionRegistry::bind_user(const SessionId session_id, const domain::UserId user_id)
{
    std::lock_guard lock(mutex_);

    const auto it = sessions_.find(session_id.value);
    if (it == sessions_.end())
        return std::nullopt;

    const auto& session = it->second;

    if (const auto existing = session->user_id()) {
        if (*existing != user_id) {
            const auto rit = session_by_user_.find(existing->value());
            if (rit != session_by_user_.end() && rit->second == session_id)
                session_by_user_.erase(rit);
        }
    }

    std::optional<SessionId> displaced;
    if (const auto rit = session_by_user_.find(user_id.value()); rit != session_by_user_.end()) {
        if (rit->second != session_id) {
            displaced = rit->second;
            clear_user_binding(*displaced);
        }
    }

    session->set_user_id(user_id);
    session_by_user_[user_id.value()] = session_id;
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
        session_by_user_.clear();
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
