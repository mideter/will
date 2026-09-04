#include "sqlite_store.h"

#include "sqlite_util.h"

#include "entities/dead_vessel.h"

#include <stdexcept>
#include <sqlite3.h>


namespace will {


SqliteStore::SqliteStore(SqliteDatabase& database)
    : database_(database)
{}


std::vector<domain::Soul> SqliteStore::load_souls()
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "SELECT id, name FROM souls;", -1, &stmt, nullptr), db,
                 "prepare load_souls");

    std::vector<domain::Soul> souls;

    int rc = sqlite3_step(stmt);
    while (rc == SQLITE_ROW) {
        const domain::id::Soul id{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))};
        const unsigned char* const name_text = sqlite3_column_text(stmt, 1);
        if (!name_text)
            throw std::runtime_error("load_souls: missing name in database");

        const auto name = domain::SoulName::parse(reinterpret_cast<const char*>(name_text));
        if (!name)
            throw std::runtime_error("load_souls: invalid name in database");

        souls.emplace_back(id, *name);
        rc = sqlite3_step(stmt);
    }

    check_sqlite(rc, db, "load_souls step");
    sqlite3_finalize(stmt);
    return souls;
}


std::vector<domain::Vessel> SqliteStore::load_vessels()
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "SELECT id, device_token, soul_id FROM vessels;", -1, &stmt, nullptr),
                 db, "prepare load_vessels");

    std::vector<domain::Vessel> vessels;

    int rc = sqlite3_step(stmt);
    while (rc == SQLITE_ROW) {
        const domain::id::Vessel id{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))};
        const char* const device_token = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const domain::id::Soul soul_id{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 2))};

        if (!device_token)
            throw std::runtime_error("load_vessels: missing device_token in database");

        try {
            vessels.emplace_back(id, domain::DeadVessel{std::string_view{device_token}}, soul_id);
        } catch (const std::invalid_argument&) {
            throw std::runtime_error("load_vessels: invalid device_token in database");
        }
        rc = sqlite3_step(stmt);
    }

    check_sqlite(rc, db, "load_vessels step");
    sqlite3_finalize(stmt);
    return vessels;
}


std::pair<domain::Soul, domain::Vessel> SqliteStore::insert_soul_with_vessel(const domain::DeadVessel& dead,
                                                                             const domain::SoulName name)
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    check_sqlite(sqlite3_exec(db, "BEGIN IMMEDIATE;", nullptr, nullptr, nullptr), db, "begin insert soul/vessel");

    sqlite3_stmt* soul_stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "INSERT INTO souls (name) VALUES (?);", -1, &soul_stmt, nullptr), db,
                 "prepare insert soul");

    check_sqlite(sqlite3_bind_text(soul_stmt, 1, name.text().data(), static_cast<int>(name.text().size()),
                                   SQLITE_TRANSIENT),
                 db, "bind name");
    check_sqlite(sqlite3_step(soul_stmt), db, "insert soul step");
    sqlite3_finalize(soul_stmt);

    const domain::id::Soul soul_id{static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db))};

    sqlite3_stmt* vessel_stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db, "INSERT INTO vessels (device_token, soul_id) VALUES (?, ?);", -1,
                                    &vessel_stmt, nullptr),
                 db, "prepare insert vessel");

    const std::string_view token_text = dead.text();
    check_sqlite(sqlite3_bind_text(vessel_stmt, 1, token_text.data(), static_cast<int>(token_text.size()),
                                   SQLITE_TRANSIENT),
                 db, "bind device_token");
    check_sqlite(sqlite3_bind_int64(vessel_stmt, 2, static_cast<sqlite3_int64>(soul_id.value())), db,
                 "bind soul_id");
    check_sqlite(sqlite3_step(vessel_stmt), db, "insert vessel step");
    sqlite3_finalize(vessel_stmt);

    const domain::id::Vessel vessel_id{static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db))};

    check_sqlite(sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr), db, "commit insert soul/vessel");

    return {domain::Soul{soul_id, name}, domain::Vessel{vessel_id, dead, soul_id}};
}


} // namespace will
