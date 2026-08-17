#pragma once

#include "auth_token.h"
#include "support/timestamp.h"
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
