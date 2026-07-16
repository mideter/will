#include "connectionaccountstore.h"

#include <utility>


namespace will {


bool ConnectionAccountStore::has(const std::uint64_t connection_id) const
{
    std::lock_guard lock(mutex_);
    return accounts_.contains(connection_id);
}


std::optional<domain::Account> ConnectionAccountStore::get(const std::uint64_t connection_id) const
{
    std::lock_guard lock(mutex_);

    const auto it = accounts_.find(connection_id);

    if (it == accounts_.end())
        return std::nullopt;

    return it->second;
}


std::optional<std::uint64_t> ConnectionAccountStore::set(const std::uint64_t connection_id,
                                                         domain::Account account)
{
    std::lock_guard lock(mutex_);

    if (const auto existing = accounts_.find(connection_id); existing != accounts_.end()) {
        if (existing->second.user_id != account.user_id) {
            const auto rit = connection_by_user_.find(existing->second.user_id.value);
            if (rit != connection_by_user_.end() && rit->second == connection_id)
                connection_by_user_.erase(rit);
        }
    }

    std::optional<std::uint64_t> displaced;
    if (const auto rit = connection_by_user_.find(account.user_id.value); rit != connection_by_user_.end()) {
        if (rit->second != connection_id) {
            displaced = rit->second;
            accounts_.erase(*displaced);
        }
    }

    const auto user_id = account.user_id.value;
    accounts_.insert_or_assign(connection_id, std::move(account));
    connection_by_user_[user_id] = connection_id;
    return displaced;
}


void ConnectionAccountStore::remove(const std::uint64_t connection_id)
{
    std::lock_guard lock(mutex_);

    const auto it = accounts_.find(connection_id);
    if (it == accounts_.end())
        return;

    const auto user_id = it->second.user_id.value;
    accounts_.erase(it);

    const auto rit = connection_by_user_.find(user_id);
    if (rit != connection_by_user_.end() && rit->second == connection_id)
        connection_by_user_.erase(rit);
}


} // namespace will
