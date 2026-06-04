#include "sqlite_message_repository_impl.h"

#include "sqlite_util.h"

#include <algorithm>
#include <sqlite3.h>


namespace will {


SqliteMessageRepositoryImpl::SqliteMessageRepositoryImpl(SqliteDatabase& database)
    : database_(database)
{}


domain::Message SqliteMessageRepositoryImpl::append(const domain::ChatId chat, const domain::UserId author,
                                                    const std::string_view body, const domain::TimestampMs ts)
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db,
                                    "INSERT INTO messages (chat_id, author_user_id, body, created_at_ms) "
                                    "VALUES (?, ?, ?, ?);",
                                    -1, &stmt, nullptr),
                 db, "prepare insert message");

    check_sqlite(sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(chat.value)), db, "bind chat_id");
    check_sqlite(sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(author.value)), db,
                 "bind author_user_id");
    check_sqlite(sqlite3_bind_text(stmt, 3, body.data(), static_cast<int>(body.size()), SQLITE_TRANSIENT),
                 db, "bind body");
    check_sqlite(sqlite3_bind_int64(stmt, 4, ts), db, "bind created_at_ms");

    check_sqlite(sqlite3_step(stmt), db, "insert message step");
    sqlite3_finalize(stmt);

    const std::uint64_t id = static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db));
    return domain::Message{id, chat, author, std::string(body), ts};
}


std::vector<domain::Message> SqliteMessageRepositoryImpl::load_last(const domain::ChatId chat,
                                                                    const std::uint32_t limit)
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db,
                                    "SELECT id, chat_id, author_user_id, body, created_at_ms FROM messages "
                                    "WHERE chat_id = ? ORDER BY id DESC LIMIT ?;",
                                    -1, &stmt, nullptr),
                 db, "prepare load_last messages");

    check_sqlite(sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(chat.value)), db, "bind chat_id");
    check_sqlite(sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(limit)), db, "bind limit");

    std::vector<domain::Message> rows;
    rows.reserve(limit);

    int rc = sqlite3_step(stmt);
    while (rc == SQLITE_ROW) {
        domain::Message row;
        row.id = static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0));
        row.chat_id = domain::ChatId{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 1))};
        row.author_id = domain::UserId{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 2))};
        row.body = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        row.created_at = sqlite3_column_int64(stmt, 4);
        rows.push_back(std::move(row));
        rc = sqlite3_step(stmt);
    }

    check_sqlite(rc, db, "load_last messages step");
    sqlite3_finalize(stmt);

    std::reverse(rows.begin(), rows.end());
    return rows;
}


} // namespace will
