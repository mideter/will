#pragma once

#include "user_id.h"
#include "user_name.h"

#include <string>


namespace will::domain {


/// Registered user account bound to a single device token.
struct User {
    UserId id{};
    std::string device_token;
    UserName name;
};


} // namespace will::domain
