#pragma once

#include "entities/user.h"
#include "entities/user_id.h"

#include <optional>
#include <string_view>


namespace will::domain {


class UserRepository {
public:
    virtual ~UserRepository() = default;

    virtual std::optional<User> find_by_device_token(std::string_view device_token) = 0;
    virtual User create_user(std::string_view device_token, std::string_view name) = 0;
    virtual void set_name(UserId id, std::string_view name) = 0;
};


} // namespace will::domain
