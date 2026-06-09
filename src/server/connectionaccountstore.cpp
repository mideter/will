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


void ConnectionAccountStore::set(const std::uint64_t connection_id, domain::Account account)
{
    std::lock_guard lock(mutex_);
    accounts_.insert_or_assign(connection_id, std::move(account));
}


void ConnectionAccountStore::remove(const std::uint64_t connection_id)
{
    std::lock_guard lock(mutex_);
    accounts_.erase(connection_id);
}


} // namespace will
