#include "sqlite_letter_repository_impl.h"

#include "sqlite_util.h"

#include <algorithm>
#include <sqlite3.h>


namespace will {


SqliteLetterRepositoryImpl::SqliteLetterRepositoryImpl(SqliteDatabase& database)
    : database_(database)
{}


domain::Letter SqliteLetterRepositoryImpl::append(const domain::id::Abode abode, const domain::id::God author,
                                                  const std::string_view body, const domain::Timestamp ts)
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db,
                                    "INSERT INTO letters (abode_id, author_god_id, body, created_at_ns) "
                                    "VALUES (?, ?, ?, ?);",
                                    -1, &stmt, nullptr),
                 db, "prepare insert letter");

    check_sqlite(sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(abode.value())), db, "bind abode_id");
    check_sqlite(sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(author.value())), db,
                 "bind author_god_id");
    check_sqlite(sqlite3_bind_text(stmt, 3, body.data(), static_cast<int>(body.size()), SQLITE_TRANSIENT),
                 db, "bind body");
    check_sqlite(sqlite3_bind_int64(stmt, 4, ts.value()), db, "bind created_at_ns");

    check_sqlite(sqlite3_step(stmt), db, "insert letter step");
    sqlite3_finalize(stmt);

    const std::uint64_t id = static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db));
    return domain::Letter{domain::id::Letter{id}, abode, author, std::string(body), ts};
}


std::vector<domain::Letter> SqliteLetterRepositoryImpl::load_last(const domain::id::Abode abode,
                                                                  const std::uint32_t limit)
{
    std::lock_guard lock(database_.mutex());

    sqlite3* const db = database_.db();
    sqlite3_stmt* stmt = nullptr;
    check_sqlite(sqlite3_prepare_v2(db,
                                    "SELECT id, abode_id, author_god_id, body, created_at_ns "
                                    "FROM letters "
                                    "WHERE abode_id = ? ORDER BY id DESC LIMIT ?;",
                                    -1, &stmt, nullptr),
                 db, "prepare load_last letters");

    check_sqlite(sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(abode.value())), db, "bind abode_id");
    check_sqlite(sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(limit)), db, "bind limit");

    std::vector<domain::Letter> rows;
    rows.reserve(limit);

    int rc = sqlite3_step(stmt);

    while (rc == SQLITE_ROW) {
        const char* const body_text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        rows.push_back(domain::Letter{
            domain::id::Letter{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))},
            domain::id::Abode{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 1))},
            domain::id::God{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 2))},
            body_text ? std::string(body_text) : std::string{},
            domain::Timestamp{sqlite3_column_int64(stmt, 4)},
        });
        rc = sqlite3_step(stmt);
    }

    check_sqlite(rc, db, "load_last letters step");
    sqlite3_finalize(stmt);

    std::reverse(rows.begin(), rows.end());
    return rows;
}


} // namespace will
