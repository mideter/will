module;

#include <cstdint>
#include <mutex>
#include <optional>
#include <sqlite3.h>
#include <string>
#include <string_view>

module will.persistence.sqlite_user_repository;

import will.persistence.sqlite_util;


namespace will {


SqliteUserRepositoryImpl::SqliteUserRepositoryImpl(SqliteDatabase& database)
    : database_(database)
{}


std::optional<domain::User> SqliteUserRepositoryImpl::find_by_device_token(const std::string_view device_token)
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "SELECT id, device_token, name FROM users WHERE device_token = ? LIMIT 1;",
                                    -1, &stmt, nullptr),
                 db, "prepare find_by_device_token");

    check_sqlite(sqlite3_bind_text(stmt, 1, device_token.data(), static_cast<int>(device_token.size()),
                                   SQLITE_TRANSIENT),
                 db, "bind device_token");

    std::optional<domain::User> user;
    const int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        std::string name;
        if (const unsigned char* raw_name = sqlite3_column_text(stmt, 2))
            name = reinterpret_cast<const char*>(raw_name);

        user = domain::User{
            .id = domain::UserId{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))},
            .device_token = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            .name = std::move(name),
        };
    }

    check_sqlite(rc == SQLITE_ROW ? SQLITE_OK : rc, db, "find_by_device_token step");
    sqlite3_finalize(stmt);
    return user;
}


domain::User SqliteUserRepositoryImpl::create_user(const std::string_view device_token,
                                                   const std::string_view name)
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "INSERT INTO users (device_token, name) VALUES (?, ?);", -1, &stmt,
                                    nullptr),
                 db, "prepare create_user");

    check_sqlite(sqlite3_bind_text(stmt, 1, device_token.data(), static_cast<int>(device_token.size()),
                                   SQLITE_TRANSIENT),
                 db, "bind device_token");
    check_sqlite(sqlite3_bind_text(stmt, 2, name.data(), static_cast<int>(name.size()), SQLITE_TRANSIENT), db,
                 "bind name");

    check_sqlite(sqlite3_step(stmt), db, "create_user step");
    sqlite3_finalize(stmt);

    const domain::UserId id{static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db))};
    return domain::User{id, std::string(device_token), std::string(name)};
}


void SqliteUserRepositoryImpl::set_name(const domain::UserId id, const std::string_view name)
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "UPDATE users SET name = ? WHERE id = ?;", -1, &stmt, nullptr), db,
                 "prepare set_name");

    check_sqlite(sqlite3_bind_text(stmt, 1, name.data(), static_cast<int>(name.size()), SQLITE_TRANSIENT), db,
                 "bind name");
    check_sqlite(sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(id.value)), db, "bind id");

    check_sqlite(sqlite3_step(stmt), db, "set_name step");
    sqlite3_finalize(stmt);
}


} // namespace will
