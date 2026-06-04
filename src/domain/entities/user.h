#pragma once

#include "user_id.h"

#include <string>


namespace will::domain {


/// Registered user account (no password material in the domain surface).
struct User {
    UserId id{};
    std::string login;
    std::string display_name;
};


} // namespace will::domain
