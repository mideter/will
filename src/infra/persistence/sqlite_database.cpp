#include "sqlite_database.h"

#include "password_hash.h"
#include "sqlite_util.h"

#include <cstring>
#include <format>
#include <sqlite3.h>


namespace will {


namespace {


constexpr const char* DevAdminLogin = "admin";
constexpr const char* DevAdminPassword = "admin";
constexpr const char* DevAdminDisplayName = "Administrator";


} // namespace


SqliteDatabase::SqliteDatabase(std::string db_path)
    : db_path_(std::move(db_path))
{
    open_database();
    init_schema();
    seed_dev_admin_user();
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
    if (table_has_column(db, "messages", "sender_ip"))
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
  login TEXT UNIQUE NOT NULL,
  password_hash TEXT NOT NULL,
  display_name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS messages (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  chat_id INTEGER NOT NULL,
  author_user_id INTEGER NOT NULL REFERENCES users(id),
  body TEXT NOT NULL,
  created_at_ms INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS auth_sessions (
  token TEXT PRIMARY KEY,
  user_id INTEGER NOT NULL,
  expires_at_ms INTEGER NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_messages_created_at ON messages(created_at_ms);
)sql";

    check_sqlite(sqlite3_exec(db_, InitSchemaSql, nullptr, nullptr, nullptr), db_, "init_schema");
}


void SqliteDatabase::seed_dev_admin_user()
{
    std::lock_guard lock(mutex_);

    sqlite3_stmt* count_stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM users;", -1, &count_stmt, nullptr),
                 db_, "prepare user count");
    check_sqlite(sqlite3_step(count_stmt), db_, "user count step");
    const int count = sqlite3_column_int(count_stmt, 0);
    sqlite3_finalize(count_stmt);

    if (count > 0)
        return;

    const std::string password_hash = hash_password(DevAdminPassword);

    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db_,
                                    "INSERT INTO users (id, login, password_hash, display_name) "
                                    "VALUES (1, ?, ?, ?);",
                                    -1, &stmt, nullptr),
                 db_, "prepare seed admin");

    check_sqlite(sqlite3_bind_text(stmt, 1, DevAdminLogin, -1, SQLITE_STATIC), db_, "bind login");
    check_sqlite(sqlite3_bind_text(stmt, 2, password_hash.c_str(), -1, SQLITE_TRANSIENT), db_,
                 "bind password_hash");
    check_sqlite(sqlite3_bind_text(stmt, 3, DevAdminDisplayName, -1, SQLITE_STATIC), db_,
                 "bind display_name");

    check_sqlite(sqlite3_step(stmt), db_, "seed admin step");
    sqlite3_finalize(stmt);
}


} // namespace will
