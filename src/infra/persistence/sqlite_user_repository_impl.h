#pragma once

#include "ports/user_repository.h"
#include "sqlite_database.h"

#include <string_view>


namespace will {


class SqliteUserRepositoryImpl final : public domain::UserRepository {
public:
    explicit SqliteUserRepositoryImpl(SqliteDatabase& database);

    std::optional<domain::User> find_by_phone(std::string_view phone) override;
    domain::User create_user(std::string_view phone) override;

private:
    SqliteDatabase& database_;
};


} // namespace will
