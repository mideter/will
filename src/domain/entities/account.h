#pragma once

// Declarations shared with module will.domain.account (account.cppm).
// Included from use-case / port headers. In new .cpp files prefer:
//   import will.domain.account;

#include "auth_token.h"
#include "timestamp.h"
#include "user_id.h"


namespace will::domain {


/// Authenticated connection context for an active session.
struct Account {
    UserId user_id{};
    AuthToken session_token;
    Timestamp authenticated_at{};
    std::string name;
};


} // namespace will::domain
