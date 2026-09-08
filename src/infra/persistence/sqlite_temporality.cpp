#include "sqlite_temporality.h"

#include "sqlite_util.h"

#include "values/device_token.h"

#include <algorithm>
#include <stdexcept>
#include <sqlite3.h>


namespace will {


SqliteTemporality::SqliteTemporality(SqliteDatabase& database)
	: database_(database)
{}


domain::Time& SqliteTemporality::time()
{
	return time_;
}


std::vector<domain::Man> SqliteTemporality::men()
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	sqlite3_stmt* stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db,
									"SELECT m.id, m.soul_id, s.name, m.vessel_id, v.device_token "
									"FROM men AS m "
									"INNER JOIN souls AS s ON s.id = m.soul_id "
									"INNER JOIN vessels AS v ON v.id = m.vessel_id;",
									-1, &stmt, nullptr),
				 db, "prepare men");

	std::vector<domain::Man> men;

	int rc = sqlite3_step(stmt);
	while (rc == SQLITE_ROW) {
		const domain::id::Man man_id{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))};
		const domain::id::Soul soul_id{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 1))};
		const unsigned char* const name_text = sqlite3_column_text(stmt, 2);
		if (!name_text)
			throw std::runtime_error("men: missing soul name in database");

		const auto name = domain::SoulName::parse(reinterpret_cast<const char*>(name_text));
		if (!name)
			throw std::runtime_error("men: invalid soul name in database");

		const domain::id::Vessel vessel_id{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 3))};
		const char* const device_token = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
		if (!device_token)
			throw std::runtime_error("men: missing device_token in database");

		const auto token = domain::DeviceToken::parse(device_token);
		if (!token)
			throw std::runtime_error("men: invalid device_token in database");

		men.emplace_back(man_id, domain::Soul{soul_id, *name}, domain::Vessel{vessel_id, *token});
		rc = sqlite3_step(stmt);
	}

	check_sqlite(rc, db, "men step");
	sqlite3_finalize(stmt);
	return men;
}


domain::Man SqliteTemporality::enroll(const domain::DeviceToken& token, const domain::SoulName name)
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteTransaction tx(db);

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
	check_sqlite(sqlite3_prepare_v2(db, "INSERT INTO vessels (device_token) VALUES (?);", -1, &vessel_stmt,
									nullptr),
				 db, "prepare insert vessel");

	const std::string_view token_text = token.text();
	check_sqlite(sqlite3_bind_text(vessel_stmt, 1, token_text.data(), static_cast<int>(token_text.size()),
								   SQLITE_TRANSIENT),
				 db, "bind device_token");
	check_sqlite(sqlite3_step(vessel_stmt), db, "insert vessel step");
	sqlite3_finalize(vessel_stmt);

	const domain::id::Vessel vessel_id{static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db))};

	sqlite3_stmt* man_stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db, "INSERT INTO men (soul_id, vessel_id) VALUES (?, ?);", -1, &man_stmt,
									nullptr),
				 db, "prepare insert man");
	check_sqlite(sqlite3_bind_int64(man_stmt, 1, static_cast<sqlite3_int64>(soul_id.value())), db,
				 "bind soul_id");
	check_sqlite(sqlite3_bind_int64(man_stmt, 2, static_cast<sqlite3_int64>(vessel_id.value())), db,
				 "bind vessel_id");
	check_sqlite(sqlite3_step(man_stmt), db, "insert man step");
	sqlite3_finalize(man_stmt);

	const domain::id::Man man_id{static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db))};

	tx.commit();

	return domain::Man{man_id, domain::Soul{soul_id, name}, domain::Vessel{vessel_id, token}};
}


void SqliteTemporality::fix(const domain::id::Abode abode, const domain::id::Soul author,
							 const std::string_view body)
{
	const domain::Timestamp ts = time_.instant();
	// Validate body before insert (Letter enforces non-empty / max length).
	const domain::Letter drafted{domain::id::Letter{1}, abode, author, std::string(body), ts};

	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	sqlite3_stmt* stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db,
									"INSERT INTO letters (abode_id, author_soul_id, body, created_at_ns) "
									"VALUES (?, ?, ?, ?);",
									-1, &stmt, nullptr),
				 db, "prepare insert letter");

	check_sqlite(sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(abode.value())), db, "bind abode_id");
	check_sqlite(sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(author.value())), db,
				 "bind author_soul_id");
	check_sqlite(sqlite3_bind_text(stmt, 3, drafted.body().data(), static_cast<int>(drafted.body().size()),
								   SQLITE_TRANSIENT),
				 db, "bind body");
	check_sqlite(sqlite3_bind_int64(stmt, 4, ts.value()), db, "bind created_at_ns");

	check_sqlite(sqlite3_step(stmt), db, "insert letter step");
	sqlite3_finalize(stmt);
}


std::vector<domain::Letter> SqliteTemporality::letters(const domain::id::Abode abode,
													  const std::uint32_t limit)
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	sqlite3_stmt* stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db,
									"SELECT id, abode_id, author_soul_id, body, created_at_ns "
									"FROM letters "
									"WHERE abode_id = ? ORDER BY id DESC LIMIT ?;",
									-1, &stmt, nullptr),
				 db, "prepare letters");

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
			domain::id::Soul{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 2))},
			body_text ? std::string(body_text) : std::string{},
			domain::Timestamp{sqlite3_column_int64(stmt, 4)},
		});
		rc = sqlite3_step(stmt);
	}

	check_sqlite(rc, db, "letters step");
	sqlite3_finalize(stmt);

	std::reverse(rows.begin(), rows.end());
	return rows;
}


} // namespace will
