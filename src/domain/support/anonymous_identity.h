#pragma once

#include "entities/account.h"
#include "entities/user_id.h"

#include <functional>
#include <string>
#include <string_view>


namespace will::domain {


/// Stage-A anonymous identity: stable {@link UserId} derived from a peer address string (e.g. IP).
inline UserId user_id_for_peer_address(std::string_view peer_address)
{
    return UserId{std::hash<std::string>{}(std::string(peer_address))};
}


inline Account anonymous_account_for_peer(std::string_view peer_address, TimestampMs authenticated_at)
{
    return Account{user_id_for_peer_address(peer_address), {}, authenticated_at};
}


} // namespace will::domain
