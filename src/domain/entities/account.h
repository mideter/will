#pragma once

#include "device_token.h"
#include "timestamp.h"
#include "user_id.h"


namespace will::domain {


/// Authenticated connection context for an active session.
struct Account {
    UserId user_id{};
    DeviceToken device_token;
    Timestamp authenticated_at{};
};


} // namespace will::domain
