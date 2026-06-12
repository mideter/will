#include "sqlite_otp_store_impl.h"

#include "sqlite_util.h"

#include <sqlite3.h>


namespace will {


SqliteOtpStoreImpl::SqliteOtpStoreImpl(SqliteDatabase& database)
    : database_(database)
{}


void SqliteOtpStoreImpl::save_challenge(domain::OtpChallenge challenge)
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db,
                                    "INSERT INTO otp_challenges (phone, code_hash, expires_at_ms, attempts, "
                                    "peer_ip) VALUES (?, ?, ?, ?, ?) "
                                    "ON CONFLICT(phone) DO UPDATE SET "
                                    "code_hash = excluded.code_hash, "
                                    "expires_at_ms = excluded.expires_at_ms, "
                                    "attempts = excluded.attempts, "
                                    "peer_ip = excluded.peer_ip;",
                                    -1, &stmt, nullptr),
                 db, "prepare save_challenge");

    check_sqlite(sqlite3_bind_text(stmt, 1, challenge.phone.c_str(),
                                   static_cast<int>(challenge.phone.size()), SQLITE_TRANSIENT),
                 db, "bind phone");
    check_sqlite(sqlite3_bind_text(stmt, 2, challenge.code_hash.c_str(),
                                   static_cast<int>(challenge.code_hash.size()), SQLITE_TRANSIENT),
                 db, "bind code_hash");
    check_sqlite(sqlite3_bind_int64(stmt, 3, challenge.expires_at_ms), db, "bind expires_at_ms");
    check_sqlite(sqlite3_bind_int(stmt, 4, challenge.attempts), db, "bind attempts");
    check_sqlite(sqlite3_bind_text(stmt, 5, challenge.peer_ip.c_str(),
                                   static_cast<int>(challenge.peer_ip.size()), SQLITE_TRANSIENT),
                 db, "bind peer_ip");

    check_sqlite(sqlite3_step(stmt), db, "save_challenge step");
    sqlite3_finalize(stmt);
}


std::optional<domain::OtpChallenge> SqliteOtpStoreImpl::find_challenge(const std::string_view phone)
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db,
                                    "SELECT phone, code_hash, expires_at_ms, attempts, peer_ip "
                                    "FROM otp_challenges WHERE phone = ? LIMIT 1;",
                                    -1, &stmt, nullptr),
                 db, "prepare find_challenge");

    check_sqlite(sqlite3_bind_text(stmt, 1, phone.data(), static_cast<int>(phone.size()), SQLITE_TRANSIENT),
                 db, "bind phone");

    std::optional<domain::OtpChallenge> challenge;
    const int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        domain::OtpChallenge row;
        row.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        row.code_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        row.expires_at_ms = sqlite3_column_int64(stmt, 2);
        row.attempts = sqlite3_column_int(stmt, 3);
        row.peer_ip = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        challenge = std::move(row);
    }

    check_sqlite(rc == SQLITE_ROW ? SQLITE_OK : rc, db, "find_challenge step");
    sqlite3_finalize(stmt);
    return challenge;
}


void SqliteOtpStoreImpl::increment_attempts(const std::string_view phone)
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db,
                                    "UPDATE otp_challenges SET attempts = attempts + 1 WHERE phone = ?;",
                                    -1, &stmt, nullptr),
                 db, "prepare increment_attempts");

    check_sqlite(sqlite3_bind_text(stmt, 1, phone.data(), static_cast<int>(phone.size()), SQLITE_TRANSIENT),
                 db, "bind phone");

    check_sqlite(sqlite3_step(stmt), db, "increment_attempts step");
    sqlite3_finalize(stmt);
}


void SqliteOtpStoreImpl::invalidate(const std::string_view phone)
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "DELETE FROM otp_challenges WHERE phone = ?;", -1, &stmt, nullptr),
                 db, "prepare invalidate");

    check_sqlite(sqlite3_bind_text(stmt, 1, phone.data(), static_cast<int>(phone.size()), SQLITE_TRANSIENT),
                 db, "bind phone");

    check_sqlite(sqlite3_step(stmt), db, "invalidate step");
    sqlite3_finalize(stmt);
}


std::size_t SqliteOtpStoreImpl::count_challenges_by_peer_ip(const std::string_view peer_ip,
                                                            const domain::TimestampMs since_ms)
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db,
                                    "SELECT COUNT(*) FROM otp_challenges "
                                    "WHERE peer_ip = ? AND expires_at_ms >= ?;",
                                    -1, &stmt, nullptr),
                 db, "prepare count_challenges_by_peer_ip");

    check_sqlite(sqlite3_bind_text(stmt, 1, peer_ip.data(), static_cast<int>(peer_ip.size()),
                                   SQLITE_TRANSIENT),
                 db, "bind peer_ip");
    check_sqlite(sqlite3_bind_int64(stmt, 2, since_ms), db, "bind since_ms");

    std::size_t count = 0;
    const int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
        count = static_cast<std::size_t>(sqlite3_column_int64(stmt, 0));

    check_sqlite(rc == SQLITE_ROW ? SQLITE_OK : rc, db, "count_challenges_by_peer_ip step");
    sqlite3_finalize(stmt);
    return count;
}


} // namespace will
