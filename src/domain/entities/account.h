#pragma once

#include "device_token.h"
#include "timestamp.h"
#include "user_id.h"


namespace will::domain {


/// Authenticated connection context for an active session.
class Account {
public:
    /// user_id must be non-zero (enforced by UserId).
    Account(UserId user_id, DeviceToken device_token, Timestamp authenticated_at);

    UserId user_id() const noexcept { return user_id_; }
    const DeviceToken& device_token() const noexcept { return device_token_; }
    Timestamp authenticated_at() const noexcept { return authenticated_at_; }

    bool operator==(const Account&) const = default;

private:
    UserId user_id_;
    DeviceToken device_token_;
    Timestamp authenticated_at_;
};


} // namespace will::domain
