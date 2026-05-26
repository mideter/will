#include "messagestore.h"

#include <algorithm>
#include <format>
#include <sqlite3.h>
#include <stdexcept>


namespace will {


namespace {


void check_sqlite(int rc, sqlite3* db, const char* context)
{
    if (rc == SQLITE_OK || rc == SQLITE_DONE || rc == SQLITE_ROW)
        return;

    const char* message = db ? sqlite3_errmsg(db) : sqlite3_errstr(rc);
    throw std::runtime_error(std::format("{}: {}", context, message ? message : "unknown error"));
}


} // namespace


MessageStore::MessageStore(std::string db_path)
    : db_path_(std::move(db_path))
{
    open_database();
    init_schema();
}


MessageStore::~MessageStore()
{
    if (db_)
        sqlite3_close(db_);
}


void MessageStore::open_database()
{
    check_sqlite(sqlite3_open_v2(db_path_.c_str(), &db_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr),
                 db_, "sqlite3_open_v2");

    check_sqlite(sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr), db_,
                 "PRAGMA journal_mode");
    check_sqlite(sqlite3_exec(db_, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr), db_,
                 "PRAGMA synchronous");
}


void MessageStore::init_schema()
{
    static constexpr const char* kSchema = R"sql(
CREATE TABLE IF NOT EXISTS messages (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    body TEXT NOT NULL,
    sender_ip TEXT NOT NULL,
    created_at_ms INTEGER NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_messages_created_at ON messages(created_at_ms);
)sql";

    check_sqlite(sqlite3_exec(db_, kSchema, nullptr, nullptr, nullptr), db_, "init_schema");
}


std::uint64_t MessageStore::insert_message(const std::string_view body, const std::string_view sender_ip,
                                           const std::int64_t created_at_ms)
{
    std::lock_guard lock(mutex_);

    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db_,
                                    "INSERT INTO messages (body, sender_ip, created_at_ms) VALUES (?, ?, ?);",
                                    -1, &stmt, nullptr),
                 db_, "prepare insert");

    check_sqlite(sqlite3_bind_text(stmt, 1, body.data(), static_cast<int>(body.size()), SQLITE_TRANSIENT),
                 db_, "bind body");
    check_sqlite(
        sqlite3_bind_text(stmt, 2, sender_ip.data(), static_cast<int>(sender_ip.size()), SQLITE_TRANSIENT),
        db_, "bind sender_ip");
    check_sqlite(sqlite3_bind_int64(stmt, 3, created_at_ms), db_, "bind created_at_ms");

    check_sqlite(sqlite3_step(stmt), db_, "insert step");
    sqlite3_finalize(stmt);

    return static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db_));
}


std::vector<StoredMessage> MessageStore::load_last(const std::uint32_t limit)
{
    std::lock_guard lock(mutex_);

    sqlite3_stmt* stmt = nullptr;
    check_sqlite(
        sqlite3_prepare_v2(
            db_,
            "SELECT id, body, sender_ip, created_at_ms FROM messages "
            "ORDER BY id DESC LIMIT ?;",
            -1, &stmt, nullptr),
        db_, "prepare load_last");

    check_sqlite(sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(limit)), db_, "bind limit");

    std::vector<StoredMessage> rows;
    rows.reserve(limit);

    int rc = sqlite3_step(stmt);
    while (rc == SQLITE_ROW) {
        StoredMessage row;
        row.id = static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0));
        row.body = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        row.sender_ip = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        row.created_at_ms = sqlite3_column_int64(stmt, 3);
        rows.push_back(std::move(row));
        rc = sqlite3_step(stmt);
    }

    check_sqlite(rc, db_, "load_last step");
    sqlite3_finalize(stmt);

    std::reverse(rows.begin(), rows.end());
    return rows;
}


} // namespace will
