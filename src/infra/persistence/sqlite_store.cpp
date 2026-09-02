#include "sqlite_store.h"

#include "sqlite_util.h"

#include <stdexcept>
#include <sqlite3.h>


namespace will {


SqliteStore::SqliteStore(SqliteDatabase& database)
    : database_(database)
{}


std::vector<domain::God> SqliteStore::load_all()
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "SELECT id, device_token, name FROM gods;", -1, &stmt, nullptr), db,
                 "prepare load_all gods");

    std::vector<domain::God> gods;

    int rc = sqlite3_step(stmt);
    while (rc == SQLITE_ROW) {
        const domain::GodId id{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))};
        const char* const device_token = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const unsigned char* const name_text = sqlite3_column_text(stmt, 2);
        if (!name_text)
            throw std::runtime_error("load_all gods: missing name in database");

        const auto name = domain::GodName::parse(reinterpret_cast<const char*>(name_text));
        if (!name)
            throw std::runtime_error("load_all gods: invalid name in database");

        const auto token = domain::DeviceToken::parse(device_token);
        if (!token)
            throw std::runtime_error("load_all gods: invalid device_token in database");

        gods.emplace_back(id, *token, *name);
        rc = sqlite3_step(stmt);
    }

    check_sqlite(rc, db, "load_all gods step");
    sqlite3_finalize(stmt);
    return gods;
}


domain::God SqliteStore::insert(const std::string_view device_token, const domain::GodName name)
{
    const auto token = domain::DeviceToken::parse(device_token);
    if (!token)
        throw std::invalid_argument("insert god: invalid device_token");

    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "INSERT INTO gods (device_token, name) VALUES (?, ?);", -1, &stmt,
                                    nullptr),
                 db, "prepare insert god");

    const std::string_view token_text = token->text();
    check_sqlite(sqlite3_bind_text(stmt, 1, token_text.data(), static_cast<int>(token_text.size()),
                                   SQLITE_TRANSIENT),
                 db, "bind device_token");
    check_sqlite(sqlite3_bind_text(stmt, 2, name.text().data(), static_cast<int>(name.text().size()),
                                   SQLITE_TRANSIENT),
                 db, "bind name");

    check_sqlite(sqlite3_step(stmt), db, "insert god step");
    sqlite3_finalize(stmt);

    const domain::GodId id{static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db))};
    return domain::God{id, *token, name};
}


} // namespace will
