#include "sqlite_user_repository_impl.h"

#include "sqlite_util.h"

#include <stdexcept>
#include <sqlite3.h>


namespace will {


SqliteUserRepositoryImpl::SqliteUserRepositoryImpl(SqliteDatabase& database)
    : database_(database)
{
    load_all();
}


void SqliteUserRepositoryImpl::load_all()
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "SELECT id, device_token, name FROM users;", -1, &stmt, nullptr), db,
                 "prepare load_all users");

    int rc = sqlite3_step(stmt);
    while (rc == SQLITE_ROW) {
        domain::User row;
        row.id = domain::UserId{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))};
        row.device_token = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (const unsigned char* name = sqlite3_column_text(stmt, 2))
            row.name = reinterpret_cast<const char*>(name);

        const auto token = domain::DeviceToken::parse(row.device_token);
        if (!token)
            throw std::runtime_error("load_all users: invalid device_token in database");

        id_by_token_[*token] = row.id;
        users_by_id_[row.id] = std::move(row);
        rc = sqlite3_step(stmt);
    }

    check_sqlite(rc, db, "load_all users step");
    sqlite3_finalize(stmt);
}


std::optional<domain::User> SqliteUserRepositoryImpl::find_by_device_token(const std::string_view device_token)
{
    const auto token = domain::DeviceToken::parse(device_token);
    if (!token)
        return std::nullopt;

    std::lock_guard lock(database_.mutex());

    const auto token_it = id_by_token_.find(*token);
    if (token_it == id_by_token_.end())
        return std::nullopt;

    const auto user_it = users_by_id_.find(token_it->second);
    if (user_it == users_by_id_.end())
        return std::nullopt;

    return user_it->second;
}


std::optional<domain::User> SqliteUserRepositoryImpl::find_by_id(const domain::UserId id)
{
    std::lock_guard lock(database_.mutex());

    const auto it = users_by_id_.find(id);
    if (it == users_by_id_.end())
        return std::nullopt;

    return it->second;
}


domain::User SqliteUserRepositoryImpl::create_user(const std::string_view device_token,
                                                   const std::string_view name)
{
    const auto token = domain::DeviceToken::parse(device_token);
    if (!token)
        throw std::invalid_argument("create_user: invalid device_token");

    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "INSERT INTO users (device_token, name) VALUES (?, ?);", -1, &stmt,
                                    nullptr),
                 db, "prepare create_user");

    const std::string_view token_text = token->text();
    check_sqlite(sqlite3_bind_text(stmt, 1, token_text.data(), static_cast<int>(token_text.size()),
                                   SQLITE_TRANSIENT),
                 db, "bind device_token");
    check_sqlite(sqlite3_bind_text(stmt, 2, name.data(), static_cast<int>(name.size()), SQLITE_TRANSIENT), db,
                 "bind name");

    check_sqlite(sqlite3_step(stmt), db, "create_user step");
    sqlite3_finalize(stmt);

    const domain::UserId id{static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db))};
    domain::User user{id, std::string(token_text), std::string(name)};
    id_by_token_[*token] = id;
    users_by_id_[id] = user;
    return user;
}


} // namespace will
