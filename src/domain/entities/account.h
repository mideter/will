#pragma once

#include "auth_token.h"
#include "message.h"
#include "user_id.h"


namespace will::domain {


/// Authenticated connection context for an active session.
struct Account {
    UserId user_id{};
    AuthToken session_token;
    TimestampMs authenticated_at = 0;
};


} // namespace will::domain
