#pragma once

#include "device_token.h"
#include "user_id.h"
#include "user_name.h"


namespace will::domain {


/// Registered user account bound to a single device token.
class User {
public:
    /// Throws std::invalid_argument if id is zero.
    User(UserId id, DeviceToken device_token, UserName name);

    UserId id() const noexcept { return id_; }
    const DeviceToken& device_token() const noexcept { return device_token_; }
    const UserName& name() const noexcept { return name_; }

    bool operator==(const User&) const = default;

private:
    UserId id_;
    DeviceToken device_token_;
    UserName name_;
};


} // namespace will::domain
