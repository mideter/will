#include "sqlite_database.h"

#include "sqlite_util.h"

#include <cstring>
#include <format>
#include <sqlite3.h>


namespace will {


SqliteDatabase::SqliteDatabase(std::string db_path)
    : db_path_(std::move(db_path))
{
    open_database();
    init_schema();
}


SqliteDatabase::~SqliteDatabase()
{
    if (db_)
        sqlite3_close(db_);
}


void SqliteDatabase::open_database()
{
    check_sqlite(sqlite3_open_v2(db_path_.c_str(), &db_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr),
                 db_, "sqlite3_open_v2");

    check_sqlite(sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr), db_,
                 "PRAGMA journal_mode");
    check_sqlite(sqlite3_exec(db_, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr), db_,
                 "PRAGMA synchronous");
}


namespace {


bool table_has_column(sqlite3* db, const char* table, const char* column)
{
    const std::string pragma = std::format("PRAGMA table_info({});", table);
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, pragma.c_str(), -1, &stmt, nullptr), db, "prepare table_info");

    bool found = false;
    int rc = sqlite3_step(stmt);
    while (rc == SQLITE_ROW) {
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (name && std::strcmp(name, column) == 0)
            found = true;
        rc = sqlite3_step(stmt);
    }

    check_sqlite(rc, db, "table_info step");
    sqlite3_finalize(stmt);
    return found;
}


bool needs_schema_reset(sqlite3* db)
{
    if (table_has_column(db, "messages", "chat_id"))
        return true;
    if (table_has_column(db, "messages", "sender_ip"))
        return true;
    if (table_has_column(db, "messages", "created_at_ms"))
        return true;
    if (table_has_column(db, "users", "password_hash"))
        return true;
    if (table_has_column(db, "users", "phone"))
        return true;

    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "SELECT name FROM sqlite_master WHERE type='table' AND name='users';",
                                    -1, &stmt, nullptr),
                 db, "prepare users table check");
    const int rc = sqlite3_step(stmt);
    const bool has_users = rc == SQLITE_ROW;
    sqlite3_finalize(stmt);
    check_sqlite(rc == SQLITE_ROW ? SQLITE_OK : rc, db, "users table check step");

    return !has_users && table_has_column(db, "messages", "body");
}


void drop_legacy_tables(sqlite3* db)
{
    static constexpr const char* DropLegacyTablesSql = R"sql(
DROP TABLE IF EXISTS messages;
DROP TABLE IF EXISTS auth_sessions;
DROP TABLE IF EXISTS otp_challenges;
DROP TABLE IF EXISTS users;
)sql";
    check_sqlite(sqlite3_exec(db, DropLegacyTablesSql, nullptr, nullptr, nullptr), db,
                 "drop_legacy_tables");
}


} // namespace


void SqliteDatabase::init_schema()
{
    if (needs_schema_reset(db_))
        drop_legacy_tables(db_);

    static constexpr const char* InitSchemaSql = R"sql(
CREATE TABLE IF NOT EXISTS users (
  id INTEGER PRIMARY KEY,
  device_token TEXT UNIQUE NOT NULL,
  name TEXT NOT NULL DEFAULT ''
);

CREATE TABLE IF NOT EXISTS messages (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  abode_id INTEGER NOT NULL,
  author_user_id INTEGER NOT NULL REFERENCES users(id),
  body TEXT NOT NULL,
  created_at_ns INTEGER NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_messages_created_at ON messages(created_at_ns);
)sql";

    check_sqlite(sqlite3_exec(db_, InitSchemaSql, nullptr, nullptr, nullptr), db_, "init_schema");

    if (table_has_column(db_, "users", "id") && !table_has_column(db_, "users", "name")) {
        check_sqlite(sqlite3_exec(db_, "ALTER TABLE users ADD COLUMN name TEXT NOT NULL DEFAULT '';", nullptr,
                                  nullptr, nullptr),
                     db_, "add users.name");
    }
}


} // namespace will
