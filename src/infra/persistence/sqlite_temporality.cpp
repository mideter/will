#include "sqlite_temporality.h"

#include "acts/tie.h"
#include "beings/novice.h"
#include "beings/soul.h"
#include "beings/testator.h"
#include "values/word.h"

#include "sqlite_util.h"

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <sqlite3.h>


namespace will {
namespace {


domain::Supplication read_pending_supplication(sqlite3_stmt* stmt)
{
	const domain::id::Soul suppliant_id{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0))};
	const domain::id::Soul addressee_id{static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 1))};
	return domain::Supplication{
		static_cast<const domain::Novice&>(domain::Soul::of(suppliant_id)),
		static_cast<const domain::Testator&>(domain::Soul::of(addressee_id)),
	};
}


const domain::Tie& live_tie(const domain::id::Tie id, const domain::id::Soul novice)
{
	const auto& place = static_cast<const domain::Novice&>(domain::Soul::of(novice)).obedience(id);
	return dynamic_cast<const domain::Tie&>(place);
}


std::uint64_t read_hwm(sqlite3* db)
{
	SqliteStmt stmt(db, "SELECT value FROM place_hwm WHERE id = 1;", "prepare read hwm");
	if (!stmt.step_row("read hwm step"))
		return 0;
	return static_cast<std::uint64_t>(stmt.column_i64(0));
}


void note_place(sqlite3* db, const std::uint64_t value)
{
	SqliteStmt stmt(db, "UPDATE place_hwm SET value = MAX(value, ?) WHERE id = 1;", "prepare note place");
	stmt.bind_i64(1, static_cast<std::int64_t>(value), "bind value");
	stmt.step_done("note place step");
}


std::uint64_t next_place_id(sqlite3* db)
{
	SqliteStmt max_tie(db, "SELECT COALESCE(MAX(id), 0) FROM obediences;", "prepare max obedience");
	std::uint64_t ties = 0;
	if (max_tie.step_row("max obedience step"))
		ties = static_cast<std::uint64_t>(max_tie.column_i64(0));

	const std::uint64_t next = std::max(read_hwm(db), ties) + 1;
	SqliteStmt bump(db, "UPDATE place_hwm SET value = ? WHERE id = 1;", "prepare bump hwm");
	bump.bind_i64(1, static_cast<std::int64_t>(next), "bind next");
	bump.step_done("bump hwm step");
	return next;
}


bool pair_exists(sqlite3* db, const domain::id::Soul testator, const domain::id::Soul novice)
{
	SqliteStmt stmt(db,
					"SELECT 1 FROM obediences "
					"WHERE testator_soul_id = ? AND novice_soul_id = ? LIMIT 1;",
					"prepare pair_exists");
	stmt.bind_i64(1, static_cast<std::int64_t>(testator.value()), "bind testator");
	stmt.bind_i64(2, static_cast<std::int64_t>(novice.value()), "bind novice");
	return stmt.step_row("pair_exists step");
}


} // namespace


SqliteTemporality::SqliteTemporality(SqliteDatabase& time_db, domain::Time& time)
	: time_db_(time_db)
	, time_(time)
{}


domain::Embodiment SqliteTemporality::embody(const domain::id::Soul soul, domain::SoulName name,
											 domain::DeviceToken token)
{
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();
	SqliteTransaction tx(db);

	note_place(db, soul.value());

	SqliteStmt vessel_stmt(db, "INSERT INTO vessels (device_token) VALUES (?);", "prepare insert vessel");
	vessel_stmt.bind_text(1, token.text(), "bind device_token");
	vessel_stmt.step_done("insert vessel step");

	const domain::id::Vessel vessel_id{sqlite_last_insert_id(db)};

	SqliteStmt man_stmt(db, "INSERT INTO men (soul_id, vessel_id, soul_name) VALUES (?, ?, ?);",
						"prepare insert man");
	man_stmt.bind_i64(1, static_cast<std::int64_t>(soul.value()), "bind soul_id");
	man_stmt.bind_i64(2, static_cast<std::int64_t>(vessel_id.value()), "bind vessel_id");
	man_stmt.bind_text(3, name.text(), "bind soul_name");
	man_stmt.step_done("insert man step");

	tx.commit();

	return domain::Embodiment{soul, std::move(name), vessel_id, std::move(token)};
}


std::vector<domain::Embodiment> SqliteTemporality::embodiments() const
{
	std::lock_guard lock(time_db_.mutex());

	sqlite3* const db = time_db_.db();
	SqliteStmt stmt(db,
					"SELECT m.soul_id, m.soul_name, m.vessel_id, v.device_token "
					"FROM men AS m "
					"INNER JOIN vessels AS v ON v.id = m.vessel_id "
					"ORDER BY m.soul_id;",
					"prepare embodiments");

	std::vector<domain::Embodiment> rows;
	while (stmt.step_row("embodiments step")) {
		const domain::id::Soul soul_id{static_cast<std::uint64_t>(stmt.column_i64(0))};
		const std::string_view name_text = stmt.column_text(1);
		if (name_text.empty())
			throw std::runtime_error("embodiments: missing soul name in database");

		const auto name = domain::SoulName::parse(name_text);
		if (!name)
			throw std::runtime_error("embodiments: invalid soul name in database");

		const domain::id::Vessel vessel_id{static_cast<std::uint64_t>(stmt.column_i64(2))};
		const std::string_view device_token = stmt.column_text(3);
		if (device_token.empty())
			throw std::runtime_error("embodiments: missing device_token in database");

		const auto token = domain::DeviceToken::parse(device_token);
		if (!token)
			throw std::runtime_error("embodiments: invalid device_token in database");

		rows.push_back(domain::Embodiment{soul_id, *name, vessel_id, *token});
	}

	return rows;
}


void SqliteTemporality::date(const domain::id::Letter id, const domain::Timestamp at)
{
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();
	SqliteStmt stmt(db, "INSERT OR REPLACE INTO datings (letter_id, created_at_ns) VALUES (?, ?);",
					"prepare date");
	stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind letter_id");
	stmt.bind_i64(2, at.value(), "bind created_at");
	stmt.step_done("date step");
}


std::vector<domain::Dating> SqliteTemporality::datings(const std::vector<domain::id::Letter>& ids) const
{
	std::vector<domain::Dating> out;
	out.reserve(ids.size());
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();
	for (const domain::id::Letter id : ids) {
		SqliteStmt stmt(db, "SELECT letter_id, created_at_ns FROM datings WHERE letter_id = ?;",
						"prepare dating");
		stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind id");
		if (stmt.step_row("dating step")) {
			out.push_back(domain::Dating{
				domain::id::Letter{static_cast<std::uint64_t>(stmt.column_i64(0))},
				domain::Timestamp{stmt.column_i64(1)},
			});
		}
	}
	return out;
}


domain::Supplication SqliteTemporality::supplicate(const domain::Novice& suppliant,
												   const domain::Testator& addressee)
{
	const domain::id::Soul suppliant_id = suppliant.Soul::id();
	const domain::id::Soul addressee_id = addressee.Soul::id();
	if (suppliant_id == addressee_id)
		throw std::invalid_argument("supplication requires distinct suppliant and addressee");

	const domain::Timestamp ts = time_.instant();
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();

	if (pair_exists(db, addressee_id, suppliant_id))
		throw std::logic_error("obedience already exists for this pair");

	{
		SqliteStmt pending_stmt(db,
								"SELECT 1 FROM supplications "
								"WHERE suppliant_soul_id = ? AND testator_soul_id = ? "
								"AND status = 'pending' LIMIT 1;",
								"prepare pending supplication");
		pending_stmt.bind_i64(1, static_cast<std::int64_t>(suppliant_id.value()), "bind suppliant");
		pending_stmt.bind_i64(2, static_cast<std::int64_t>(addressee_id.value()), "bind addressee");
		if (pending_stmt.step_row("pending step"))
			throw std::logic_error("pending supplication already exists for this pair");
	}

	SqliteStmt stmt(db,
					"INSERT INTO supplications "
					"(suppliant_soul_id, testator_soul_id, status, created_at_ns) "
					"VALUES (?, ?, ?, ?);",
					"prepare insert supplication");
	stmt.bind_i64(1, static_cast<std::int64_t>(suppliant_id.value()), "bind suppliant");
	stmt.bind_i64(2, static_cast<std::int64_t>(addressee_id.value()), "bind addressee");
	stmt.bind_text(3, "pending", "bind status");
	stmt.bind_i64(4, ts.value(), "bind created_at");
	stmt.step_done("insert supplication step");

	return domain::Supplication{suppliant, addressee};
}


std::vector<domain::Supplication> SqliteTemporality::pending_supplications(
	const domain::id::Soul addressee) const
{
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();
	SqliteStmt stmt(db,
					"SELECT suppliant_soul_id, testator_soul_id "
					"FROM supplications "
					"WHERE testator_soul_id = ? AND status = 'pending' ORDER BY id;",
					"prepare pending_supplications");
	stmt.bind_i64(1, static_cast<std::int64_t>(addressee.value()), "bind addressee");

	std::vector<domain::Supplication> rows;
	while (stmt.step_row("pending_supplications step"))
		rows.push_back(read_pending_supplication(stmt.get()));

	return rows;
}


domain::Tying SqliteTemporality::accept(const domain::Supplication& ask)
{
	const domain::Timestamp ts = time_.instant();
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();
	SqliteTransaction tx(db);

	domain::Supplication row = [&] {
		SqliteStmt load(db,
						"SELECT suppliant_soul_id, testator_soul_id "
						"FROM supplications "
						"WHERE suppliant_soul_id = ? AND testator_soul_id = ? "
						"AND status = 'pending' LIMIT 1;",
						"prepare load pending supplication");
		load.bind_i64(1, static_cast<std::int64_t>(ask.suppliant().Soul::id().value()), "bind suppliant");
		load.bind_i64(2, static_cast<std::int64_t>(ask.addressee().Soul::id().value()), "bind addressee");
		if (!load.step_row("load pending step"))
			throw std::invalid_argument("unknown supplication");

		return read_pending_supplication(load.get());
	}();

	if (pair_exists(db, row.addressee().Soul::id(), row.suppliant().Soul::id()))
		throw std::logic_error("obedience already exists for this pair");

	{
		SqliteStmt upd(db,
					   "UPDATE supplications SET status = 'accepted' "
					   "WHERE suppliant_soul_id = ? AND testator_soul_id = ? "
					   "AND status = 'pending';",
					   "prepare accept status");
		upd.bind_i64(1, static_cast<std::int64_t>(row.suppliant().Soul::id().value()), "bind suppliant");
		upd.bind_i64(2, static_cast<std::int64_t>(row.addressee().Soul::id().value()), "bind addressee");
		upd.step_done("accept status step");
	}

	const domain::id::Tie oid{next_place_id(db)};
	SqliteStmt ins(db,
				   "INSERT INTO obediences "
				   "(id, testator_soul_id, novice_soul_id, created_at_ns, seceded_at_ns) "
				   "VALUES (?, ?, ?, ?, NULL);",
				   "prepare insert obedience");
	ins.bind_i64(1, static_cast<std::int64_t>(oid.value()), "bind id");
	ins.bind_i64(2, static_cast<std::int64_t>(row.addressee().Soul::id().value()), "bind testator");
	ins.bind_i64(3, static_cast<std::int64_t>(row.suppliant().Soul::id().value()), "bind novice");
	ins.bind_i64(4, ts.value(), "bind created_at");
	ins.step_done("insert obedience step");

	tx.commit();

	return domain::Tying{oid, row.addressee().Soul::id(), row.suppliant().Soul::id()};
}


void SqliteTemporality::reject(const domain::Supplication& ask)
{
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();

	SqliteStmt upd(db,
				   "UPDATE supplications SET status = 'rejected' "
				   "WHERE suppliant_soul_id = ? AND testator_soul_id = ? "
				   "AND status = 'pending';",
				   "prepare reject");
	upd.bind_i64(1, static_cast<std::int64_t>(ask.suppliant().Soul::id().value()), "bind suppliant");
	upd.bind_i64(2, static_cast<std::int64_t>(ask.addressee().Soul::id().value()), "bind addressee");
	upd.step_done("reject step");

	if (sqlite3_changes(db) != 1)
		throw std::invalid_argument("unknown supplication");
}


domain::Tying SqliteTemporality::tying(const domain::id::Tie id) const
{
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();
	SqliteStmt stmt(db,
					"SELECT id, testator_soul_id, novice_soul_id "
					"FROM obediences WHERE id = ?;",
					"prepare tying");
	stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind id");
	if (!stmt.step_row("tying step"))
		throw std::invalid_argument("unknown obedience");

	return domain::Tying{
		domain::id::Tie{static_cast<std::uint64_t>(stmt.column_i64(0))},
		domain::id::Soul{static_cast<std::uint64_t>(stmt.column_i64(1))},
		domain::id::Soul{static_cast<std::uint64_t>(stmt.column_i64(2))},
	};
}


std::vector<domain::Tying> SqliteTemporality::tyings() const
{
	std::vector<domain::Tying> out;
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();
	SqliteStmt stmt(db,
					"SELECT id, testator_soul_id, novice_soul_id FROM obediences ORDER BY id;",
					"prepare tyings");
	while (stmt.step_row("tyings step")) {
		out.push_back(domain::Tying{
			domain::id::Tie{static_cast<std::uint64_t>(stmt.column_i64(0))},
			domain::id::Soul{static_cast<std::uint64_t>(stmt.column_i64(1))},
			domain::id::Soul{static_cast<std::uint64_t>(stmt.column_i64(2))},
		});
	}
	return out;
}


domain::Deed SqliteTemporality::will(const domain::Obedience& obedience,
									  const domain::Soul& testator, const domain::Word& word)
{
	if (obedience.testator().Soul::id() != testator.id())
		throw std::logic_error("only the testator may will in this obedience");

	const domain::Timestamp ts = time_.instant();
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();

	{
		SqliteStmt known(db, "SELECT 1 FROM obediences WHERE id = ?;", "prepare known obedience");
		known.bind_i64(1, static_cast<std::int64_t>(obedience.id().value()), "bind oid");
		if (!known.step_row("known obedience step"))
			throw std::invalid_argument("unknown obedience");
	}

	SqliteStmt stmt(db,
					"INSERT INTO deeds "
					"(obedience_id, testator_soul_id, novice_soul_id, body, "
					"created_at_ns, executed_at_ns, cancelled_at_ns) "
					"VALUES (?, ?, ?, ?, ?, NULL, NULL);",
					"prepare insert deed");
	stmt.bind_i64(1, static_cast<std::int64_t>(obedience.id().value()), "bind obedience_id");
	stmt.bind_i64(2, static_cast<std::int64_t>(obedience.testator().Soul::id().value()), "bind testator");
	stmt.bind_i64(3, static_cast<std::int64_t>(obedience.novice().Soul::id().value()), "bind novice");
	stmt.bind_text(4, word.body(), "bind body");
	stmt.bind_i64(5, ts.value(), "bind created_at");
	stmt.step_done("insert deed step");

	const domain::id::Deed did{sqlite_last_insert_id(db)};

	return domain::Deed{did, dynamic_cast<const domain::Tie&>(obedience), word, ts};
}


domain::Deed SqliteTemporality::execute(const domain::Deed& deed)
{
	const domain::Timestamp ts = time_.instant();
	domain::id::Tie oid{1};
	domain::Word word{std::string{deed.body()}};
	domain::Timestamp created = deed.created_at();
	domain::id::Deed did{deed.id().value()};

	{
		std::lock_guard lock(time_db_.mutex());
		sqlite3* const db = time_db_.db();

		{
			SqliteStmt load(db,
							"SELECT obedience_id, executed_at_ns, cancelled_at_ns "
							"FROM deeds WHERE id = ?;",
							"prepare load deed");
			load.bind_i64(1, static_cast<std::int64_t>(deed.id().value()), "bind id");
			if (!load.step_row("load deed step"))
				throw std::invalid_argument("unknown deed");
			oid = domain::id::Tie{static_cast<std::uint64_t>(load.column_i64(0))};
			const bool open = load.column_is_null(1) && load.column_is_null(2);
			if (!open)
				throw std::logic_error("deed is not open");
		}

		{
			SqliteStmt known(db, "SELECT 1 FROM obediences WHERE id = ?;", "prepare known obedience");
			known.bind_i64(1, static_cast<std::int64_t>(oid.value()), "bind oid");
			if (!known.step_row("known obedience step"))
				throw std::invalid_argument("unknown obedience");
		}

		SqliteStmt upd(db, "UPDATE deeds SET executed_at_ns = ? WHERE id = ?;", "prepare execute");
		upd.bind_i64(1, ts.value(), "bind executed_at");
		upd.bind_i64(2, static_cast<std::int64_t>(deed.id().value()), "bind id");
		upd.step_done("execute step");
	}

	const domain::Tying kept = tying(oid);
	return domain::Deed{did, live_tie(kept.id(), kept.novice()), word, created, ts, std::nullopt};
}


domain::Deed SqliteTemporality::deed(const domain::id::Deed id) const
{
	domain::id::Tie oid{1};
	std::string body_str;
	domain::Timestamp created{0};
	std::optional<domain::Timestamp> executed;
	std::optional<domain::Timestamp> cancelled;
	domain::id::Deed did{1};

	{
		std::lock_guard lock(time_db_.mutex());
		sqlite3* const db = time_db_.db();
		SqliteStmt stmt(db,
						"SELECT id, obedience_id, body, created_at_ns, executed_at_ns, "
						"cancelled_at_ns FROM deeds WHERE id = ?;",
						"prepare deed");
		stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind id");
		if (!stmt.step_row("deed step"))
			throw std::invalid_argument("unknown deed");

		did = domain::id::Deed{static_cast<std::uint64_t>(stmt.column_i64(0))};
		oid = domain::id::Tie{static_cast<std::uint64_t>(stmt.column_i64(1))};
		body_str = std::string(stmt.column_text(2));
		created = domain::Timestamp{stmt.column_i64(3)};
		if (!stmt.column_is_null(4))
			executed = domain::Timestamp{stmt.column_i64(4)};
		if (!stmt.column_is_null(5))
			cancelled = domain::Timestamp{stmt.column_i64(5)};
	}

	const domain::Tying kept = tying(oid);
	return domain::Deed{did, live_tie(kept.id(), kept.novice()), domain::Word{body_str}, created,
						executed, cancelled};
}


std::vector<domain::Deed> SqliteTemporality::deeds(const domain::id::Tie tie) const
{
	std::vector<domain::id::Deed> ids;
	{
		std::lock_guard lock(time_db_.mutex());
		sqlite3* const db = time_db_.db();
		SqliteStmt stmt(db, "SELECT id FROM deeds WHERE obedience_id = ? ORDER BY id;",
						"prepare deeds");
		stmt.bind_i64(1, static_cast<std::int64_t>(tie.value()), "bind oid");
		while (stmt.step_row("deeds step"))
			ids.push_back(domain::id::Deed{static_cast<std::uint64_t>(stmt.column_i64(0))});
	}

	std::vector<domain::Deed> out;
	out.reserve(ids.size());

	for (const domain::id::Deed id : ids)
		out.push_back(this->deed(id));

	return out;
}


} // namespace will
