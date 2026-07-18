#include "sqlite_user_repository_impl.h"

#include "sqlite_util.h"

#include <sqlite3.h>


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
        domain::User row;
        row.id = domain::UserId{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))};
        row.device_token = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (const unsigned char* name = sqlite3_column_text(stmt, 2))
            row.name = reinterpret_cast<const char*>(name);
        user = std::move(row);
    }

    check_sqlite(rc == SQLITE_ROW ? SQLITE_OK : rc, db, "find_by_device_token step");
    sqlite3_finalize(stmt);
    return user;
}


domain::User SqliteUserRepositoryImpl::create_user(const std::string_view device_token)
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "INSERT INTO users (device_token) VALUES (?);", -1, &stmt, nullptr), db,
                 "prepare create_user");

    check_sqlite(sqlite3_bind_text(stmt, 1, device_token.data(), static_cast<int>(device_token.size()),
                                   SQLITE_TRANSIENT),
                 db, "bind device_token");

    check_sqlite(sqlite3_step(stmt), db, "create_user step");
    sqlite3_finalize(stmt);

    const domain::UserId id{static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db))};
    return domain::User{id, std::string(device_token), {}};
}


} // namespace will
