#include "sqlite_auth_session_store_impl.h"

#include "sqlite_util.h"

#include <chrono>
#include <format>
#include <random>
#include <sqlite3.h>


namespace will {


namespace {


constexpr domain::TimestampMs SessionLifetimeMs = 24 * 60 * 60 * 1000;


domain::TimestampMs now_ms()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}


std::string generate_session_token()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<std::uint64_t> dist;
    return std::format("{:016x}{:016x}", dist(gen), dist(gen));
}


} // namespace


SqliteAuthSessionStoreImpl::SqliteAuthSessionStoreImpl(SqliteDatabase& database)
    : database_(database)
{}


domain::AuthToken SqliteAuthSessionStoreImpl::issue_session(const domain::UserId user)
{
    const domain::TimestampMs issued_at = now_ms();
    const domain::TimestampMs expires_at = issued_at + SessionLifetimeMs;
    const domain::AuthToken token{generate_session_token()};

    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db,
                                    "INSERT INTO auth_sessions (token, user_id, expires_at_ms) VALUES (?, ?, ?);",
                                    -1, &stmt, nullptr),
                 db, "prepare issue_session");

    check_sqlite(sqlite3_bind_text(stmt, 1, token.value.c_str(), static_cast<int>(token.value.size()),
                                   SQLITE_TRANSIENT),
                 db, "bind token");
    check_sqlite(sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(user.value)), db, "bind user_id");
    check_sqlite(sqlite3_bind_int64(stmt, 3, expires_at), db, "bind expires_at_ms");

    check_sqlite(sqlite3_step(stmt), db, "issue_session step");
    sqlite3_finalize(stmt);

    return token;
}


std::optional<domain::Account> SqliteAuthSessionStoreImpl::resolve_token(const domain::AuthToken token)
{
    if (token.empty())
        return std::nullopt;

    const domain::TimestampMs now = now_ms();

    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db,
                                    "SELECT user_id, expires_at_ms FROM auth_sessions "
                                    "WHERE token = ? LIMIT 1;",
                                    -1, &stmt, nullptr),
                 db, "prepare resolve_token");

    check_sqlite(sqlite3_bind_text(stmt, 1, token.value.c_str(), static_cast<int>(token.value.size()),
                                   SQLITE_TRANSIENT),
                 db, "bind token");

    std::optional<domain::Account> account;
    const int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const domain::UserId user_id{
            static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))};
        const domain::TimestampMs expires_at = sqlite3_column_int64(stmt, 1);
        if (expires_at > now) {
            const domain::TimestampMs authenticated_at = expires_at - SessionLifetimeMs;
            account = domain::Account{user_id, token, authenticated_at};
        }
    }

    check_sqlite(rc == SQLITE_ROW ? SQLITE_OK : rc, db, "resolve_token step");
    sqlite3_finalize(stmt);
    return account;
}


void SqliteAuthSessionStoreImpl::revoke(const domain::AuthToken token)
{
    if (token.empty())
        return;

    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "DELETE FROM auth_sessions WHERE token = ?;", -1, &stmt, nullptr),
                 db, "prepare revoke session");

    check_sqlite(sqlite3_bind_text(stmt, 1, token.value.c_str(), static_cast<int>(token.value.size()),
                                   SQLITE_TRANSIENT),
                 db, "bind token");

    check_sqlite(sqlite3_step(stmt), db, "revoke session step");
    sqlite3_finalize(stmt);
}


} // namespace will
