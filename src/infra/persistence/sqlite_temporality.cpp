#include "sqlite_temporality.h"

#include "entities/soul.h"
#include "entities/spirit.h"
#include "values/abode_name.h"
#include "values/word.h"

#include "sqlite_util.h"

#include "values/device_token.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sqlite3.h>


namespace will {
namespace {


const char* status_text(const domain::SupplicationStatus status)
{
	switch (status) {
	case domain::SupplicationStatus::pending:
		return "pending";
	case domain::SupplicationStatus::accepted:
		return "accepted";
	case domain::SupplicationStatus::refused:
		return "refused";
	case domain::SupplicationStatus::withdrawn:
		return "withdrawn";
	}
	throw std::logic_error("unknown supplication status");
}


domain::SupplicationStatus status_from_text(const char* text)
{
	if (text && std::strcmp(text, "pending") == 0)
		return domain::SupplicationStatus::pending;
	if (text && std::strcmp(text, "accepted") == 0)
		return domain::SupplicationStatus::accepted;
	if (text && std::strcmp(text, "refused") == 0)
		return domain::SupplicationStatus::refused;
	if (text && std::strcmp(text, "withdrawn") == 0)
		return domain::SupplicationStatus::withdrawn;
	throw std::runtime_error("supplications: invalid status in database");
}


domain::Supplication read_supplication(sqlite3_stmt* stmt)
{
	const domain::id::Soul suppliant_id{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 1))};
	const domain::id::Soul addressee_id{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 2))};
	const domain::Spirit reach;
	domain::Heaven& heaven = reach.heaven();
	return domain::Supplication{
		domain::id::Supplication{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))},
		heaven.soul(suppliant_id),
		heaven.soul(addressee_id),
		status_from_text(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))),
		domain::Timestamp{sqlite3_column_int64(stmt, 4)},
	};
}


std::uint64_t next_place_id(sqlite3* db)
{
	sqlite3_stmt* stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db,
									"SELECT COALESCE(MAX(id), 0) FROM ("
									"  SELECT id FROM abodes"
									"  UNION ALL SELECT id FROM obediences"
									"  UNION ALL SELECT id FROM men"
									");",
									-1, &stmt, nullptr),
				 db, "prepare next_place_id");
	const int rc = sqlite3_step(stmt);
	check_sqlite(rc == SQLITE_ROW ? SQLITE_OK : rc, db, "next_place_id step");
	const auto value = static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0));
	sqlite3_finalize(stmt);
	return value + 1;
}


bool living_pair_exists(sqlite3* db, const domain::id::Soul testator, const domain::id::Soul executor)
{
	sqlite3_stmt* stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db,
									"SELECT 1 FROM obediences "
									"WHERE testator_soul_id = ? AND executor_soul_id = ? "
									"AND seceded_at_ns IS NULL LIMIT 1;",
									-1, &stmt, nullptr),
				 db, "prepare living_pair");
	check_sqlite(sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(testator.value())), db,
				 "bind testator");
	check_sqlite(sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(executor.value())), db,
				 "bind executor");
	const int rc = sqlite3_step(stmt);
	const bool exists = rc == SQLITE_ROW;
	sqlite3_finalize(stmt);
	check_sqlite(rc == SQLITE_ROW || rc == SQLITE_DONE ? SQLITE_OK : rc, db, "living_pair step");
	return exists;
}


} // namespace


SqliteTemporality::SqliteTemporality(SqliteDatabase& database)
	: database_(database)
{}


domain::Time& SqliteTemporality::time()
{
	return time_;
}


std::vector<domain::Soul> SqliteTemporality::souls()
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	sqlite3_stmt* stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db, "SELECT id, name FROM souls ORDER BY id;", -1, &stmt, nullptr), db,
				 "prepare souls");

	std::vector<domain::Soul> souls;

	int rc = sqlite3_step(stmt);
	while (rc == SQLITE_ROW) {
		const domain::id::Soul soul_id{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))};
		const unsigned char* const name_text = sqlite3_column_text(stmt, 1);
		if (!name_text)
			throw std::runtime_error("souls: missing name in database");

		const auto name = domain::SoulName::parse(reinterpret_cast<const char*>(name_text));
		if (!name)
			throw std::runtime_error("souls: invalid name in database");

		souls.emplace_back(soul_id, *name);
		rc = sqlite3_step(stmt);
	}

	check_sqlite(rc, db, "souls step");
	sqlite3_finalize(stmt);
	return souls;
}


domain::Soul SqliteTemporality::enroll(const domain::SoulName name)
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	sqlite3_stmt* soul_stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db, "INSERT INTO souls (name) VALUES (?);", -1, &soul_stmt, nullptr), db,
				 "prepare insert soul");

	check_sqlite(sqlite3_bind_text(soul_stmt, 1, name.text().data(), static_cast<int>(name.text().size()),
								   SQLITE_TRANSIENT),
				 db, "bind name");
	check_sqlite(sqlite3_step(soul_stmt), db, "insert soul step");
	sqlite3_finalize(soul_stmt);

	const domain::id::Soul soul_id{static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db))};
	return domain::Soul{soul_id, name};
}


domain::Embodiment SqliteTemporality::embody(const domain::id::Soul soul, domain::DeviceToken token)
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteTransaction tx(db);

	sqlite3_stmt* name_stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db, "SELECT name FROM souls WHERE id = ?;", -1, &name_stmt, nullptr), db,
				 "prepare soul name");
	check_sqlite(sqlite3_bind_int64(name_stmt, 1, static_cast<sqlite3_int64>(soul.value())), db, "bind soul");
	const int name_rc = sqlite3_step(name_stmt);
	if (name_rc != SQLITE_ROW) {
		sqlite3_finalize(name_stmt);
		throw std::invalid_argument("unknown soul");
	}
	const unsigned char* const name_text = sqlite3_column_text(name_stmt, 0);
	if (!name_text) {
		sqlite3_finalize(name_stmt);
		throw std::runtime_error("souls: missing name in database");
	}
	const auto name = domain::SoulName::parse(reinterpret_cast<const char*>(name_text));
	sqlite3_finalize(name_stmt);
	if (!name)
		throw std::runtime_error("souls: invalid name in database");

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
	check_sqlite(sqlite3_bind_int64(man_stmt, 1, static_cast<sqlite3_int64>(soul.value())), db, "bind soul_id");
	check_sqlite(sqlite3_bind_int64(man_stmt, 2, static_cast<sqlite3_int64>(vessel_id.value())), db,
				 "bind vessel_id");
	check_sqlite(sqlite3_step(man_stmt), db, "insert man step");
	sqlite3_finalize(man_stmt);

	const domain::id::Man man_id{static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db))};

	tx.commit();

	return domain::Embodiment{man_id, soul, *name, vessel_id, std::move(token)};
}


std::vector<domain::Embodiment> SqliteTemporality::embodiments() const
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	sqlite3_stmt* stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db,
									"SELECT m.id, m.soul_id, s.name, m.vessel_id, v.device_token "
									"FROM men AS m "
									"INNER JOIN souls AS s ON s.id = m.soul_id "
									"INNER JOIN vessels AS v ON v.id = m.vessel_id "
									"ORDER BY m.id;",
									-1, &stmt, nullptr),
				 db, "prepare embodiments");

	std::vector<domain::Embodiment> rows;

	int rc = sqlite3_step(stmt);
	while (rc == SQLITE_ROW) {
		const domain::id::Man man_id{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))};
		const domain::id::Soul soul_id{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 1))};
		const unsigned char* const name_text = sqlite3_column_text(stmt, 2);
		if (!name_text)
			throw std::runtime_error("embodiments: missing soul name in database");

		const auto name = domain::SoulName::parse(reinterpret_cast<const char*>(name_text));
		if (!name)
			throw std::runtime_error("embodiments: invalid soul name in database");

		const domain::id::Vessel vessel_id{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 3))};
		const char* const device_token = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
		if (!device_token)
			throw std::runtime_error("embodiments: missing device_token in database");

		const auto token = domain::DeviceToken::parse(device_token);
		if (!token)
			throw std::runtime_error("embodiments: invalid device_token in database");

		rows.push_back(domain::Embodiment{man_id, soul_id, *name, vessel_id, *token});
		rc = sqlite3_step(stmt);
	}

	check_sqlite(rc, db, "embodiments step");
	sqlite3_finalize(stmt);
	return rows;
}


std::vector<domain::Abode> SqliteTemporality::abodes()
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	sqlite3_stmt* stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db, "SELECT id, name FROM abodes ORDER BY id;", -1, &stmt, nullptr),
				 db, "prepare abodes");

	std::vector<domain::Abode> abodes;

	int rc = sqlite3_step(stmt);
	while (rc == SQLITE_ROW) {
		const domain::id::Abode id{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))};
		const unsigned char* const name_text = sqlite3_column_text(stmt, 1);
		if (!name_text)
			throw std::runtime_error("abodes: missing name in database");

		abodes.emplace_back(id, domain::AbodeName{reinterpret_cast<const char*>(name_text)});
		rc = sqlite3_step(stmt);
	}

	check_sqlite(rc, db, "abodes step");
	sqlite3_finalize(stmt);
	return abodes;
}


void SqliteTemporality::keep(const domain::id::Abode id, domain::AbodeName name)
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	sqlite3_stmt* stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db,
									"INSERT OR IGNORE INTO abodes (id, name) VALUES (?, ?);",
									-1, &stmt, nullptr),
				 db, "prepare keep abode");

	check_sqlite(sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(id.value())), db, "bind abode id");
	check_sqlite(sqlite3_bind_text(stmt, 2, std::string(name.text()).c_str(), -1, SQLITE_TRANSIENT), db,
				 "bind abode name");
	check_sqlite(sqlite3_step(stmt), db, "keep abode step");
	sqlite3_finalize(stmt);
}


void SqliteTemporality::join_abode(const domain::id::Abode abode, const domain::id::Man man)
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	sqlite3_stmt* stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db,
									"INSERT OR IGNORE INTO abode_men (abode_id, man_id) VALUES (?, ?);",
									-1, &stmt, nullptr),
				 db, "prepare join abode_men");

	check_sqlite(sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(abode.value())), db, "bind abode_id");
	check_sqlite(sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(man.value())), db, "bind man_id");
	check_sqlite(sqlite3_step(stmt), db, "insert abode_men step");
	sqlite3_finalize(stmt);
}


std::vector<std::pair<domain::id::Abode, domain::id::Man>> SqliteTemporality::abode_men()
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	sqlite3_stmt* stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db, "SELECT abode_id, man_id FROM abode_men ORDER BY abode_id, man_id;",
									-1, &stmt, nullptr),
				 db, "prepare abode_men");

	std::vector<std::pair<domain::id::Abode, domain::id::Man>> rows;

	int rc = sqlite3_step(stmt);
	while (rc == SQLITE_ROW) {
		rows.emplace_back(
			domain::id::Abode{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))},
			domain::id::Man{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 1))});
		rc = sqlite3_step(stmt);
	}

	check_sqlite(rc, db, "abode_men step");
	sqlite3_finalize(stmt);
	return rows;
}


void SqliteTemporality::fix(const domain::id::Place place, const domain::id::Soul author,
							 const domain::Word& word) const
{
	const domain::Timestamp ts = time_.instant();
	const domain::Letter drafted{domain::id::Letter{1}, place, author, word, ts};

	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	sqlite3_stmt* stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db,
									"INSERT INTO letters (place_id, author_soul_id, body, created_at_ns) "
									"VALUES (?, ?, ?, ?);",
									-1, &stmt, nullptr),
				 db, "prepare insert letter");

	check_sqlite(sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(place.value())), db, "bind place_id");
	check_sqlite(sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(author.value())), db,
				 "bind author_soul_id");
	check_sqlite(sqlite3_bind_text(stmt, 3, drafted.body().data(), static_cast<int>(drafted.body().size()),
								   SQLITE_TRANSIENT),
				 db, "bind body");
	check_sqlite(sqlite3_bind_int64(stmt, 4, ts.value()), db, "bind created_at_ns");

	check_sqlite(sqlite3_step(stmt), db, "insert letter step");
	sqlite3_finalize(stmt);
}


std::vector<domain::Letter> SqliteTemporality::letters(const domain::id::Place place,
													  const std::uint32_t limit) const
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	sqlite3_stmt* stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db,
									"SELECT id, place_id, author_soul_id, body, created_at_ns "
									"FROM letters "
									"WHERE place_id = ? ORDER BY id DESC LIMIT ?;",
									-1, &stmt, nullptr),
				 db, "prepare letters");

	check_sqlite(sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(place.value())), db, "bind place_id");
	check_sqlite(sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(limit)), db, "bind limit");

	std::vector<domain::Letter> rows;
	rows.reserve(limit);

	int rc = sqlite3_step(stmt);

	while (rc == SQLITE_ROW) {
		const char* const body_text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
		rows.push_back(domain::Letter{
			domain::id::Letter{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))},
			domain::id::Place{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 1))},
			domain::id::Soul{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 2))},
			domain::Word{std::string(body_text ? body_text : "")},
			domain::Timestamp{sqlite3_column_int64(stmt, 4)},
		});
		rc = sqlite3_step(stmt);
	}

	check_sqlite(rc, db, "letters step");
	sqlite3_finalize(stmt);

	std::reverse(rows.begin(), rows.end());
	return rows;
}


domain::Supplication SqliteTemporality::supplicate(const domain::Soul& suppliant,
												   const domain::Soul& addressee)
{
	const domain::id::Soul suppliant_id = suppliant.id();
	const domain::id::Soul addressee_id = addressee.id();
	if (suppliant_id == addressee_id)
		throw std::invalid_argument("supplication requires distinct suppliant and addressee");

	const domain::Timestamp ts = time_.instant();
	std::lock_guard lock(database_.mutex());
	sqlite3* const db = database_.db();

	if (living_pair_exists(db, addressee_id, suppliant_id))
		throw std::logic_error("living obedience already exists for this pair");

	sqlite3_stmt* pending_stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db,
									"SELECT 1 FROM supplications "
									"WHERE suppliant_soul_id = ? AND addressee_soul_id = ? "
									"AND status = 'pending' LIMIT 1;",
									-1, &pending_stmt, nullptr),
				 db, "prepare pending supplication");
	check_sqlite(sqlite3_bind_int64(pending_stmt, 1, static_cast<sqlite3_int64>(suppliant_id.value())), db,
				 "bind suppliant");
	check_sqlite(sqlite3_bind_int64(pending_stmt, 2, static_cast<sqlite3_int64>(addressee_id.value())), db,
				 "bind addressee");
	const int pending_rc = sqlite3_step(pending_stmt);
	const bool pending_exists = pending_rc == SQLITE_ROW;
	sqlite3_finalize(pending_stmt);
	check_sqlite(pending_rc == SQLITE_ROW || pending_rc == SQLITE_DONE ? SQLITE_OK : pending_rc, db,
				 "pending step");
	if (pending_exists)
		throw std::logic_error("pending supplication already exists for this pair");

	sqlite3_stmt* stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db,
									"INSERT INTO supplications "
									"(suppliant_soul_id, addressee_soul_id, status, created_at_ns) "
									"VALUES (?, ?, ?, ?);",
									-1, &stmt, nullptr),
				 db, "prepare insert supplication");
	check_sqlite(sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(suppliant_id.value())), db,
				 "bind suppliant");
	check_sqlite(sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(addressee_id.value())), db,
				 "bind addressee");
	check_sqlite(sqlite3_bind_text(stmt, 3, status_text(domain::SupplicationStatus::pending), -1,
								   SQLITE_STATIC),
				 db, "bind status");
	check_sqlite(sqlite3_bind_int64(stmt, 4, ts.value()), db, "bind created_at");
	check_sqlite(sqlite3_step(stmt), db, "insert supplication step");
	sqlite3_finalize(stmt);

	return domain::Supplication{
		domain::id::Supplication{static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db))},
		suppliant,
		addressee,
		domain::SupplicationStatus::pending,
		ts,
	};
}


std::vector<domain::Supplication> SqliteTemporality::pending_supplications(
	const domain::id::Soul addressee) const
{
	std::lock_guard lock(database_.mutex());
	sqlite3* const db = database_.db();
	sqlite3_stmt* stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db,
									"SELECT id, suppliant_soul_id, addressee_soul_id, status, created_at_ns "
									"FROM supplications "
									"WHERE addressee_soul_id = ? AND status = 'pending' ORDER BY id;",
									-1, &stmt, nullptr),
				 db, "prepare pending_supplications");
	check_sqlite(sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(addressee.value())), db,
				 "bind addressee");

	std::vector<domain::Supplication> rows;
	int rc = sqlite3_step(stmt);
	while (rc == SQLITE_ROW) {
		rows.push_back(read_supplication(stmt));
		rc = sqlite3_step(stmt);
	}
	check_sqlite(rc, db, "pending_supplications step");
	sqlite3_finalize(stmt);
	return rows;
}


domain::Obedience SqliteTemporality::accept(const domain::id::Supplication id)
{
	const domain::Timestamp ts = time_.instant();
	std::lock_guard lock(database_.mutex());
	sqlite3* const db = database_.db();
	SqliteTransaction tx(db);

	sqlite3_stmt* load = nullptr;
	check_sqlite(sqlite3_prepare_v2(db,
									"SELECT id, suppliant_soul_id, addressee_soul_id, status, created_at_ns "
									"FROM supplications WHERE id = ?;",
									-1, &load, nullptr),
				 db, "prepare load supplication");
	check_sqlite(sqlite3_bind_int64(load, 1, static_cast<sqlite3_int64>(id.value())), db, "bind id");
	const int load_rc = sqlite3_step(load);
	if (load_rc != SQLITE_ROW) {
		sqlite3_finalize(load);
		throw std::invalid_argument("unknown supplication");
	}
	const auto row = read_supplication(load);
	sqlite3_finalize(load);

	if (row.status() != domain::SupplicationStatus::pending)
		throw std::logic_error("supplication is not pending");
	if (living_pair_exists(db, row.addressee().id(), row.suppliant().id()))
		throw std::logic_error("living obedience already exists for this pair");

	sqlite3_stmt* upd = nullptr;
	check_sqlite(sqlite3_prepare_v2(db, "UPDATE supplications SET status = ? WHERE id = ?;", -1, &upd,
									nullptr),
				 db, "prepare accept status");
	check_sqlite(sqlite3_bind_text(upd, 1, status_text(domain::SupplicationStatus::accepted), -1,
								   SQLITE_STATIC),
				 db, "bind status");
	check_sqlite(sqlite3_bind_int64(upd, 2, static_cast<sqlite3_int64>(id.value())), db, "bind id");
	check_sqlite(sqlite3_step(upd), db, "accept status step");
	sqlite3_finalize(upd);

	const domain::id::Obedience oid{next_place_id(db)};
	sqlite3_stmt* ins = nullptr;
	check_sqlite(sqlite3_prepare_v2(db,
									"INSERT INTO obediences "
									"(id, testator_soul_id, executor_soul_id, created_at_ns, seceded_at_ns) "
									"VALUES (?, ?, ?, ?, NULL);",
									-1, &ins, nullptr),
				 db, "prepare insert obedience");
	check_sqlite(sqlite3_bind_int64(ins, 1, static_cast<sqlite3_int64>(oid.value())), db, "bind id");
	check_sqlite(sqlite3_bind_int64(ins, 2, static_cast<sqlite3_int64>(row.addressee().id().value())), db,
				 "bind testator");
	check_sqlite(sqlite3_bind_int64(ins, 3, static_cast<sqlite3_int64>(row.suppliant().id().value())), db,
				 "bind executor");
	check_sqlite(sqlite3_bind_int64(ins, 4, ts.value()), db, "bind created_at");
	check_sqlite(sqlite3_step(ins), db, "insert obedience step");
	sqlite3_finalize(ins);

	tx.commit();
	return domain::Obedience{oid, row.addressee().id(), row.suppliant().id(), true};
}


void SqliteTemporality::refuse(const domain::id::Supplication id)
{
	std::lock_guard lock(database_.mutex());
	sqlite3* const db = database_.db();

	sqlite3_stmt* load = nullptr;
	check_sqlite(sqlite3_prepare_v2(db, "SELECT status FROM supplications WHERE id = ?;", -1, &load,
									nullptr),
				 db, "prepare load status");
	check_sqlite(sqlite3_bind_int64(load, 1, static_cast<sqlite3_int64>(id.value())), db, "bind id");
	const int load_rc = sqlite3_step(load);
	if (load_rc != SQLITE_ROW) {
		sqlite3_finalize(load);
		throw std::invalid_argument("unknown supplication");
	}
	const auto status = status_from_text(reinterpret_cast<const char*>(sqlite3_column_text(load, 0)));
	sqlite3_finalize(load);
	if (status != domain::SupplicationStatus::pending)
		throw std::logic_error("supplication is not pending");

	sqlite3_stmt* upd = nullptr;
	check_sqlite(sqlite3_prepare_v2(db, "UPDATE supplications SET status = ? WHERE id = ?;", -1, &upd,
									nullptr),
				 db, "prepare refuse");
	check_sqlite(sqlite3_bind_text(upd, 1, status_text(domain::SupplicationStatus::refused), -1,
								   SQLITE_STATIC),
				 db, "bind status");
	check_sqlite(sqlite3_bind_int64(upd, 2, static_cast<sqlite3_int64>(id.value())), db, "bind id");
	check_sqlite(sqlite3_step(upd), db, "refuse step");
	sqlite3_finalize(upd);
}


domain::Obedience SqliteTemporality::obedience(const domain::id::Obedience id) const
{
	std::lock_guard lock(database_.mutex());
	sqlite3* const db = database_.db();
	sqlite3_stmt* stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db,
									"SELECT id, testator_soul_id, executor_soul_id, seceded_at_ns "
									"FROM obediences WHERE id = ?;",
									-1, &stmt, nullptr),
				 db, "prepare obedience");
	check_sqlite(sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(id.value())), db, "bind id");
	const int rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW) {
		sqlite3_finalize(stmt);
		throw std::invalid_argument("unknown obedience");
	}
	domain::Obedience out{
		domain::id::Obedience{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))},
		domain::id::Soul{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 1))},
		domain::id::Soul{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 2))},
		sqlite3_column_type(stmt, 3) == SQLITE_NULL,
	};
	sqlite3_finalize(stmt);
	return out;
}


void SqliteTemporality::secede(const domain::id::Obedience id)
{
	const domain::Timestamp ts = time_.instant();
	std::lock_guard lock(database_.mutex());
	sqlite3* const db = database_.db();

	sqlite3_stmt* load = nullptr;
	check_sqlite(sqlite3_prepare_v2(db, "SELECT seceded_at_ns FROM obediences WHERE id = ?;", -1, &load,
									nullptr),
				 db, "prepare load obedience");
	check_sqlite(sqlite3_bind_int64(load, 1, static_cast<sqlite3_int64>(id.value())), db, "bind id");
	const int load_rc = sqlite3_step(load);
	if (load_rc != SQLITE_ROW) {
		sqlite3_finalize(load);
		throw std::invalid_argument("unknown obedience");
	}
	const bool living = sqlite3_column_type(load, 0) == SQLITE_NULL;
	sqlite3_finalize(load);
	if (!living)
		throw std::logic_error("obedience is not living");

	sqlite3_stmt* upd = nullptr;
	check_sqlite(sqlite3_prepare_v2(db, "UPDATE obediences SET seceded_at_ns = ? WHERE id = ?;", -1, &upd,
									nullptr),
				 db, "prepare secede");
	check_sqlite(sqlite3_bind_int64(upd, 1, ts.value()), db, "bind seceded_at");
	check_sqlite(sqlite3_bind_int64(upd, 2, static_cast<sqlite3_int64>(id.value())), db, "bind id");
	check_sqlite(sqlite3_step(upd), db, "secede step");
	sqlite3_finalize(upd);
}


} // namespace will
