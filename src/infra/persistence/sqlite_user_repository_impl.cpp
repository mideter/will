#include "sqlite_user_repository_impl.h"

#include "password_hash.h"
#include "sqlite_util.h"

#include <sqlite3.h>


namespace will {


SqliteUserRepositoryImpl::SqliteUserRepositoryImpl(SqliteDatabase& database)
    : database_(database)
{}


std::optional<domain::User> SqliteUserRepositoryImpl::find_by_login(const std::string_view login)
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db,
                                    "SELECT id, login, display_name FROM users WHERE login = ? LIMIT 1;",
                                    -1, &stmt, nullptr),
                 db, "prepare find_by_login");

    check_sqlite(sqlite3_bind_text(stmt, 1, login.data(), static_cast<int>(login.size()), SQLITE_TRANSIENT),
                 db, "bind login");

    std::optional<domain::User> user;
    const int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        domain::User row;
        row.id = domain::UserId{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))};
        row.login = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        row.display_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user = std::move(row);
    }

    check_sqlite(rc == SQLITE_ROW ? SQLITE_OK : rc, db, "find_by_login step");
    sqlite3_finalize(stmt);
    return user;
}


bool SqliteUserRepositoryImpl::verify_password(const domain::UserId id, const std::string_view password)
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "SELECT password_hash FROM users WHERE id = ? LIMIT 1;", -1,
                                    &stmt, nullptr),
                 db, "prepare verify_password");

    check_sqlite(sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(id.value)), db, "bind user id");

    bool ok = false;
    const int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const char* stored = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        ok = stored && verify_password_hash(stored, password);
    }

    check_sqlite(rc == SQLITE_ROW ? SQLITE_OK : rc, db, "verify_password step");
    sqlite3_finalize(stmt);
    return ok;
}


} // namespace will
