#include "sessionregistry.h"

#include <utility>


namespace will {


SessionStream::SessionStream(const std::uint64_t id, grpc::ServerContext* context, Stream* stream,
                             std::string peer_address)
    : id_(id)
    , context_(context)
    , stream_(stream)
    , peer_address_(std::move(peer_address))
{}


bool SessionStream::write(const v1::ServerEvent& event)
{
    std::lock_guard lock(write_mutex_);
    if (closed_.load())
        return false;

    return stream_->Write(event);
}


void SessionStream::request_close()
{
    grpc::ServerContext* context = nullptr;
    {
        std::lock_guard lock(write_mutex_);
        if (closed_.exchange(true))
            return;
        context = context_;
        context_ = nullptr;
    }

    if (context != nullptr)
        context->TryCancel();
}


std::string SessionRegistry::peer_from_context(grpc::ServerContext* context)
{
    if (context == nullptr)
        return {};

    return std::string(context->peer());
}


std::shared_ptr<SessionStream> SessionRegistry::register_session(grpc::ServerContext* context,
                                                                 SessionStream::Stream* stream)
{
    const std::uint64_t id = next_id_.fetch_add(1);
    auto session = std::make_shared<SessionStream>(id, context, stream, peer_from_context(context));

    std::lock_guard lock(mutex_);
    sessions_.emplace(id, session);
    return session;
}


void SessionRegistry::unregister_session(const std::uint64_t session_id)
{
    std::lock_guard lock(mutex_);
    sessions_.erase(session_id);
}


void SessionRegistry::close_session(const std::uint64_t session_id)
{
    std::shared_ptr<SessionStream> session;
    {
        std::lock_guard lock(mutex_);
        const auto it = sessions_.find(session_id);
        if (it == sessions_.end())
            return;
        session = it->second;
        sessions_.erase(it);
    }

    session->request_close();
}


void SessionRegistry::enqueue_event(const std::uint64_t session_id, const v1::ServerEvent& event)
{
    std::shared_ptr<SessionStream> session;
    {
        std::lock_guard lock(mutex_);
        const auto it = sessions_.find(session_id);
        if (it == sessions_.end())
            return;
        session = it->second;
    }

    if (!session->write(event))
        close_session(session_id);
}


void SessionRegistry::broadcast_except(const std::uint64_t except_session_id, const v1::ServerEvent& event)
{
    std::vector<std::shared_ptr<SessionStream>> targets;
    {
        std::lock_guard lock(mutex_);
        targets.reserve(sessions_.size());
        for (const auto& [id, session] : sessions_) {
            if (id != except_session_id)
                targets.push_back(session);
        }
    }

    for (const auto& session : targets) {
        if (!session->write(event))
            close_session(session->id());
    }
}


std::string_view SessionRegistry::peer_address(const std::uint64_t session_id) const
{
    std::lock_guard lock(mutex_);
    const auto it = sessions_.find(session_id);
    if (it == sessions_.end())
        return {};
    return it->second->peer_address();
}


void SessionRegistry::close_all_sessions()
{
    std::vector<std::shared_ptr<SessionStream>> sessions;
    {
        std::lock_guard lock(mutex_);
        sessions.reserve(sessions_.size());
        for (auto& [id, session] : sessions_)
            sessions.push_back(session);
        sessions_.clear();
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
