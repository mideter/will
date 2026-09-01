#include "connectionaccountstore.h"

#include <utility>


namespace will {


bool ConnectionAccountStore::has(const std::uint64_t connection_id) const
{
    std::lock_guard lock(mutex_);
    return users_.contains(connection_id);
}


std::optional<domain::User> ConnectionAccountStore::get(const std::uint64_t connection_id) const
{
    std::lock_guard lock(mutex_);

    const auto it = users_.find(connection_id);

    if (it == users_.end())
        return std::nullopt;

    return it->second;
}


std::optional<std::uint64_t> ConnectionAccountStore::set(const std::uint64_t connection_id, domain::User user)
{
    std::lock_guard lock(mutex_);

    if (const auto existing = users_.find(connection_id); existing != users_.end()) {
        if (existing->second.id() != user.id()) {
            const auto rit = connection_by_user_.find(existing->second.id().value());
            if (rit != connection_by_user_.end() && rit->second == connection_id)
                connection_by_user_.erase(rit);
        }
    }

    std::optional<std::uint64_t> displaced;
    if (const auto rit = connection_by_user_.find(user.id().value()); rit != connection_by_user_.end()) {
        if (rit->second != connection_id) {
            displaced = rit->second;
            users_.erase(*displaced);
        }
    }

    const auto user_id = user.id().value();
    users_.insert_or_assign(connection_id, std::move(user));
    connection_by_user_[user_id] = connection_id;
    return displaced;
}


void ConnectionAccountStore::remove(const std::uint64_t connection_id)
{
    std::lock_guard lock(mutex_);

    const auto it = users_.find(connection_id);
    if (it == users_.end())
        return;

    const auto user_id = it->second.id().value();
    users_.erase(it);

    const auto rit = connection_by_user_.find(user_id);
    if (rit != connection_by_user_.end() && rit->second == connection_id)
        connection_by_user_.erase(rit);
}


} // namespace will
