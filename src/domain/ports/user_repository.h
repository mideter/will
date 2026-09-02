#pragma once

#include "entities/user.h"
#include "ids/user_id.h"
#include "entities/user_name.h"

#include <optional>
#include <string_view>


namespace will::domain {


class UserRepository {
public:
    virtual ~UserRepository() = default;

    virtual std::optional<User> find_by_device_token(std::string_view device_token) = 0;
    virtual std::optional<User> find_by_id(UserId id) = 0;
    virtual User create_user(std::string_view device_token, UserName name) = 0;
};


} // namespace will::domain
