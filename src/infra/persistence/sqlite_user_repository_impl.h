#pragma once

#include "ports/user_repository.h"
#include "sqlite_database.h"

#include <string_view>


namespace will {


class SqliteUserRepositoryImpl final : public domain::UserRepository {
public:
    explicit SqliteUserRepositoryImpl(SqliteDatabase& database);

    std::optional<domain::User> find_by_device_token(std::string_view device_token) override;
    std::optional<domain::User> find_by_id(domain::UserId id) override;
    domain::User create_user(std::string_view device_token, std::string_view name) override;

private:
    SqliteDatabase& database_;
};


} // namespace will
