module;

#include <string>

export module will.domain.user;

export import will.domain.user_id;

export namespace will::domain {


/// Registered user account bound to a single device token.
struct User {
    UserId id{};
    std::string device_token;
    std::string name;
};


} // namespace will::domain
