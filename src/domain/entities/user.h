#pragma once

#include "user_id.h"

#include <string>


namespace will::domain {


/// Registered user account bound to a single device token.
struct User {
    UserId id{};
    std::string device_token;
    std::string name;
};


} // namespace will::domain
