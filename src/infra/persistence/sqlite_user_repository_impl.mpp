module;

#include <optional>
#include <string_view>
#include <vector>
#include <cstdint>

export module will.persistence.sqlite_user_repository;

export import will.persistence.sqlite_database;
export import will.domain.user_repository;

export namespace will {


class SqliteUserRepositoryImpl final : public domain::UserRepository {
public:
    explicit SqliteUserRepositoryImpl(SqliteDatabase& database);

    std::optional<domain::User> find_by_device_token(std::string_view device_token) override;
    domain::User create_user(std::string_view device_token, std::string_view name) override;
    void set_name(domain::UserId id, std::string_view name) override;

private:
    SqliteDatabase& database_;
};


} // namespace will
