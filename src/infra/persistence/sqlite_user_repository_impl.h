#pragma once

#include "ports/user_repository.h"
#include "sqlite_database.h"

#include <string>
#include <string_view>
#include <unordered_map>


namespace will {


class SqliteUserRepositoryImpl final : public domain::UserRepository {
public:
    explicit SqliteUserRepositoryImpl(SqliteDatabase& database);

    std::optional<domain::User> find_by_device_token(std::string_view device_token) override;
    std::optional<domain::User> find_by_id(domain::UserId id) override;
    domain::User create_user(std::string_view device_token, std::string_view name) override;

private:
    void load_all();

    SqliteDatabase& database_;
    std::unordered_map<domain::UserId, domain::User> users_by_id_;
    std::unordered_map<std::string, domain::UserId> id_by_token_;
};


} // namespace will
