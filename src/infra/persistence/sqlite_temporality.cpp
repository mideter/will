#include "sqlite_temporality.h"

#include "beings/space.h"

#include "sqlite_util.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <sqlite3.h>


namespace will {
namespace {


bool pair_exists(sqlite3* db, const domain::id::Soul testator, const domain::id::Soul novice)
{
	SqliteStmt stmt(db,
					"SELECT 1 FROM tyings "
					"WHERE testator_soul_id = ? AND novice_soul_id = ? LIMIT 1;",
					"prepare pair_exists");
	stmt.bind_i64(1, static_cast<std::int64_t>(testator.value()), "bind testator");
	stmt.bind_i64(2, static_cast<std::int64_t>(novice.value()), "bind novice");
	return stmt.step_row("pair_exists step");
}


/// Row id of the asking of this pair that has no rejection kept, if any.
std::optional<std::int64_t>
unrejected_asking(sqlite3* db, const domain::id::Soul suppliant, const domain::id::Soul addressee)
{
	SqliteStmt stmt(db,
					"SELECT a.id FROM askings AS a "
					"WHERE a.suppliant_soul_id = ? AND a.addressee_soul_id = ? "
					"AND NOT EXISTS (SELECT 1 FROM rejections AS r WHERE r.asking_id = a.id) "
					"LIMIT 1;",
					"prepare unrejected asking");
	stmt.bind_i64(1, static_cast<std::int64_t>(suppliant.value()), "bind suppliant");
	stmt.bind_i64(2, static_cast<std::int64_t>(addressee.value()), "bind addressee");
	if (!stmt.step_row("unrejected asking step"))
		return std::nullopt;

	return stmt.column_i64(0);
}


void insert_tying(sqlite3* db, const domain::Tying& tying, const domain::Timestamp ts)
{
	SqliteStmt ins(db,
				   "INSERT INTO tyings "
				   "(id, testator_soul_id, novice_soul_id, created_at_ns) "
				   "VALUES (?, ?, ?, ?);",
				   "prepare insert tying");
	ins.bind_i64(1, static_cast<std::int64_t>(tying.id().value()), "bind id");
	ins.bind_i64(2, static_cast<std::int64_t>(tying.testator().value()), "bind testator");
	ins.bind_i64(3, static_cast<std::int64_t>(tying.novice().value()), "bind novice");
	ins.bind_i64(4, ts.value(), "bind created_at");
	ins.step_done("insert tying step");
}


} // namespace


SqliteTemporality::SqliteTemporality(SqliteDatabase& time_db, domain::Eternity& eternity)
	: time_db_(time_db)
	, eternity_(eternity)
{}


domain::Embodiment SqliteTemporality::embody(const domain::id::Soul soul, domain::SoulName name,
											 domain::DeviceToken token)
{
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();
	SqliteTransaction tx(db);

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


domain::Dating SqliteTemporality::date(const domain::id::Word id)
{
	const domain::Timestamp at = eternity_.time().instant();
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();
	SqliteStmt stmt(db, "INSERT OR REPLACE INTO datings (word_id, created_at_ns) VALUES (?, ?);",
					"prepare date");
	stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind word_id");
	stmt.bind_i64(2, at.value(), "bind created_at");
	stmt.step_done("date step");

	return domain::Dating{id, at};
}


std::vector<domain::Dating> SqliteTemporality::datings(const std::vector<domain::id::Word>& ids) const
{
	std::vector<domain::Dating> out;
	out.reserve(ids.size());
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();
	for (const domain::id::Word id : ids) {
		SqliteStmt stmt(db, "SELECT word_id, created_at_ns FROM datings WHERE word_id = ?;",
						"prepare dating");
		stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind id");
		if (stmt.step_row("dating step")) {
			out.push_back(domain::Dating{
				domain::id::Word{static_cast<std::uint64_t>(stmt.column_i64(0))},
				domain::Timestamp{stmt.column_i64(1)},
			});
		}
	}
	return out;
}


domain::Asking SqliteTemporality::ask(const domain::id::Soul suppliant,
									  const domain::id::Soul addressee)
{
	const domain::Asking asking{suppliant, addressee};
	const domain::Timestamp at = eternity_.time().instant();
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();

	if (pair_exists(db, addressee, suppliant))
		throw std::logic_error("obedience already exists for this pair");
	if (unrejected_asking(db, suppliant, addressee))
		throw std::logic_error("pending supplication already exists for this pair");

	SqliteStmt stmt(db,
					"INSERT INTO askings (suppliant_soul_id, addressee_soul_id, asked_at_ns) "
					"VALUES (?, ?, ?);",
					"prepare ask");
	stmt.bind_i64(1, static_cast<std::int64_t>(suppliant.value()), "bind suppliant");
	stmt.bind_i64(2, static_cast<std::int64_t>(addressee.value()), "bind addressee");
	stmt.bind_i64(3, at.value(), "bind asked_at");
	stmt.step_done("ask step");

	return asking;
}


std::vector<domain::Asking> SqliteTemporality::askings(const domain::id::Soul addressee) const
{
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();
	SqliteStmt stmt(db,
					"SELECT a.suppliant_soul_id, a.addressee_soul_id FROM askings AS a "
					"WHERE a.addressee_soul_id = ? "
					"AND NOT EXISTS (SELECT 1 FROM rejections AS r WHERE r.asking_id = a.id) "
					"AND NOT EXISTS (SELECT 1 FROM tyings AS o "
					"WHERE o.testator_soul_id = a.addressee_soul_id "
					"AND o.novice_soul_id = a.suppliant_soul_id) "
					"ORDER BY a.id;",
					"prepare askings");
	stmt.bind_i64(1, static_cast<std::int64_t>(addressee.value()), "bind addressee");

	std::vector<domain::Asking> rows;
	while (stmt.step_row("askings step")) {
		rows.push_back(domain::Asking{
			domain::id::Soul{static_cast<std::uint64_t>(stmt.column_i64(0))},
			domain::id::Soul{static_cast<std::uint64_t>(stmt.column_i64(1))},
		});
	}

	return rows;
}


void SqliteTemporality::reject(const domain::id::Soul suppliant, const domain::id::Soul addressee)
{
	const domain::Timestamp at = eternity_.time().instant();
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();

	const std::optional<std::int64_t> asking = unrejected_asking(db, suppliant, addressee);
	if (!asking || pair_exists(db, addressee, suppliant))
		throw std::invalid_argument("unknown supplication");

	SqliteStmt stmt(db, "INSERT INTO rejections (asking_id, rejected_at_ns) VALUES (?, ?);",
					"prepare reject");
	stmt.bind_i64(1, *asking, "bind asking_id");
	stmt.bind_i64(2, at.value(), "bind rejected_at");
	stmt.step_done("reject step");
}


domain::Tying SqliteTemporality::tie(const domain::id::Soul testator, const domain::id::Soul novice)
{
	const domain::Timestamp at = eternity_.time().instant();
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();

	if (pair_exists(db, testator, novice))
		throw std::logic_error("obedience already exists for this pair");

	const domain::Tying tying{domain::id::Tie{eternity_.space().point()}, testator, novice};
	insert_tying(db, tying, at);

	return tying;
}


std::vector<domain::Tying> SqliteTemporality::tyings() const
{
	std::vector<domain::Tying> out;
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();
	SqliteStmt stmt(db,
					"SELECT id, testator_soul_id, novice_soul_id FROM tyings ORDER BY id;",
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


void SqliteTemporality::execute(const domain::id::Word deed)
{
	const domain::Timestamp at = eternity_.time().instant();
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();

	{
		SqliteStmt kept(db, "SELECT 1 FROM executions WHERE word_id = ?;", "prepare kept execution");
		kept.bind_i64(1, static_cast<std::int64_t>(deed.value()), "bind word_id");
		if (kept.step_row("kept execution step"))
			throw std::logic_error("deed is already executed");
	}

	SqliteStmt stmt(db, "INSERT INTO executions (word_id, executed_at_ns) VALUES (?, ?);",
					"prepare execute");
	stmt.bind_i64(1, static_cast<std::int64_t>(deed.value()), "bind word_id");
	stmt.bind_i64(2, at.value(), "bind executed_at");
	stmt.step_done("execute step");
}


std::vector<domain::Execution>
SqliteTemporality::executions(const std::vector<domain::id::Word>& ids) const
{
	std::vector<domain::Execution> out;
	out.reserve(ids.size());
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();
	for (const domain::id::Word id : ids) {
		SqliteStmt stmt(db, "SELECT word_id, executed_at_ns FROM executions WHERE word_id = ?;",
						"prepare execution");
		stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind id");
		if (stmt.step_row("execution step")) {
			out.push_back(domain::Execution{
				domain::id::Word{static_cast<std::uint64_t>(stmt.column_i64(0))},
				domain::Timestamp{stmt.column_i64(1)},
			});
		}
	}
	return out;
}


} // namespace will
