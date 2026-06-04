#pragma once

#include "ports/user_repository.h"
#include "sqlite_database.h"

#include <string_view>


namespace will {


class SqliteUserRepositoryImpl final : public domain::UserRepository {
public:
    explicit SqliteUserRepositoryImpl(SqliteDatabase& database);

    std::optional<domain::User> find_by_login(std::string_view login) override;
    bool verify_password(domain::UserId id, std::string_view password) override;

private:
    SqliteDatabase& database_;
};


} // namespace will
