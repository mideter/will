#include "sqlite_store.h"

#include "sqlite_util.h"

#include <stdexcept>
#include <sqlite3.h>


namespace will {


SqliteStore::SqliteStore(SqliteDatabase& database)
    : database_(database)
{}


std::vector<domain::God> SqliteStore::load_gods()
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "SELECT id, name FROM gods;", -1, &stmt, nullptr), db,
                 "prepare load_gods");

    std::vector<domain::God> gods;

    int rc = sqlite3_step(stmt);
    while (rc == SQLITE_ROW) {
        const domain::id::God id{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))};
        const unsigned char* const name_text = sqlite3_column_text(stmt, 1);
        if (!name_text)
            throw std::runtime_error("load_gods: missing name in database");

        const auto name = domain::GodName::parse(reinterpret_cast<const char*>(name_text));
        if (!name)
            throw std::runtime_error("load_gods: invalid name in database");

        gods.emplace_back(id, *name);
        rc = sqlite3_step(stmt);
    }

    check_sqlite(rc, db, "load_gods step");
    sqlite3_finalize(stmt);
    return gods;
}


std::vector<domain::Vessel> SqliteStore::load_vessels()
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "SELECT id, device_token, god_id FROM vessels;", -1, &stmt, nullptr),
                 db, "prepare load_vessels");

    std::vector<domain::Vessel> vessels;

    int rc = sqlite3_step(stmt);
    while (rc == SQLITE_ROW) {
        const domain::id::Vessel id{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))};
        const char* const device_token = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const domain::id::God god_id{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 2))};

        const auto token = domain::DeviceToken::parse(device_token);
        if (!token)
            throw std::runtime_error("load_vessels: invalid device_token in database");

        vessels.emplace_back(id, *token, god_id);
        rc = sqlite3_step(stmt);
    }

    check_sqlite(rc, db, "load_vessels step");
    sqlite3_finalize(stmt);
    return vessels;
}


std::pair<domain::God, domain::Vessel> SqliteStore::insert_god_with_vessel(const std::string_view device_token,
                                                                             const domain::GodName name)
{
    const auto token = domain::DeviceToken::parse(device_token);
    if (!token)
        throw std::invalid_argument("insert_god_with_vessel: invalid device_token");

    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    check_sqlite(sqlite3_exec(db, "BEGIN IMMEDIATE;", nullptr, nullptr, nullptr), db, "begin insert god/vessel");

    sqlite3_stmt* god_stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "INSERT INTO gods (name) VALUES (?);", -1, &god_stmt, nullptr), db,
                 "prepare insert god");

    check_sqlite(sqlite3_bind_text(god_stmt, 1, name.text().data(), static_cast<int>(name.text().size()),
                                   SQLITE_TRANSIENT),
                 db, "bind name");
    check_sqlite(sqlite3_step(god_stmt), db, "insert god step");
    sqlite3_finalize(god_stmt);

    const domain::id::God god_id{static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db))};

    sqlite3_stmt* vessel_stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "INSERT INTO vessels (device_token, god_id) VALUES (?, ?);", -1,
                                    &vessel_stmt, nullptr),
                 db, "prepare insert vessel");

    const std::string_view token_text = token->text();
    check_sqlite(sqlite3_bind_text(vessel_stmt, 1, token_text.data(), static_cast<int>(token_text.size()),
                                   SQLITE_TRANSIENT),
                 db, "bind device_token");
    check_sqlite(sqlite3_bind_int64(vessel_stmt, 2, static_cast<sqlite3_int64>(god_id.value())), db,
                 "bind god_id");
    check_sqlite(sqlite3_step(vessel_stmt), db, "insert vessel step");
    sqlite3_finalize(vessel_stmt);

    const domain::id::Vessel vessel_id{static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db))};

    check_sqlite(sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr), db, "commit insert god/vessel");

    return {domain::God{god_id, name}, domain::Vessel{vessel_id, *token, god_id}};
}


} // namespace will
