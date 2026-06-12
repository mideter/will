#include "sqlite_user_repository_impl.h"

#include "sqlite_util.h"

#include <sqlite3.h>


namespace will {


SqliteUserRepositoryImpl::SqliteUserRepositoryImpl(SqliteDatabase& database)
    : database_(database)
{}


std::optional<domain::User> SqliteUserRepositoryImpl::find_by_phone(const std::string_view phone)
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "SELECT id, phone FROM users WHERE phone = ? LIMIT 1;", -1, &stmt,
                                    nullptr),
                 db, "prepare find_by_phone");

    check_sqlite(sqlite3_bind_text(stmt, 1, phone.data(), static_cast<int>(phone.size()), SQLITE_TRANSIENT),
                 db, "bind phone");

    std::optional<domain::User> user;
    const int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        domain::User row;
        row.id = domain::UserId{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))};
        row.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user = std::move(row);
    }

    check_sqlite(rc == SQLITE_ROW ? SQLITE_OK : rc, db, "find_by_phone step");
    sqlite3_finalize(stmt);
    return user;
}


domain::User SqliteUserRepositoryImpl::create_user(const std::string_view phone)
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "INSERT INTO users (phone) VALUES (?);", -1, &stmt, nullptr), db,
                 "prepare create_user");

    check_sqlite(sqlite3_bind_text(stmt, 1, phone.data(), static_cast<int>(phone.size()), SQLITE_TRANSIENT),
                 db, "bind phone");

    check_sqlite(sqlite3_step(stmt), db, "create_user step");
    sqlite3_finalize(stmt);

    const domain::UserId id{static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db))};
    return domain::User{id, std::string(phone)};
}


} // namespace will
