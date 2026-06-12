#pragma once

#include "entities/user.h"
#include "entities/user_id.h"

#include <optional>
#include <string_view>


namespace will::domain {


class UserRepository {
public:
    virtual ~UserRepository() = default;

    virtual std::optional<User> find_by_phone(std::string_view phone) = 0;
    virtual User create_user(std::string_view phone) = 0;
};


} // namespace will::domain
