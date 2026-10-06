#include "sqlite_temporality.h"

#include "sqlite_util.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <sqlite3.h>


namespace will {
namespace {


/// Row id of the supplication of this pair awaiting its answer, if any.
std::optional<std::int64_t>
awaiting_supplication(sqlite3* db, const domain::id::Soul suppliant, const domain::id::Soul addressee)
{
	SqliteStmt stmt(db,
					"SELECT a.id FROM supplications AS a "
					"WHERE a.suppliant_soul_id = ? AND a.addressee_soul_id = ? "
					"AND NOT EXISTS (SELECT 1 FROM answers AS r WHERE r.supplication_id = a.id) "
					"LIMIT 1;",
					"prepare awaiting supplication");
	stmt.bind_i64(1, static_cast<std::int64_t>(suppliant.value()), "bind suppliant");
	stmt.bind_i64(2, static_cast<std::int64_t>(addressee.value()), "bind addressee");
	if (!stmt.step_row("awaiting supplication step"))
		return std::nullopt;

	return stmt.column_i64(0);
}


} // namespace


SqliteTemporality::SqliteTemporality(std::string path, domain::Eternity& eternity)
	: time_db_(std::move(path), SqliteFace::Temporality)
	, eternity_(eternity)
{}


domain::matter::Vessel SqliteTemporality::form(domain::DeviceToken token)
{
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();

	SqliteStmt stmt(db, "INSERT INTO vessels (device_token) VALUES (?);", "prepare keep vessel");
	stmt.bind_text(1, token.text(), "bind device_token");
	stmt.step_done("keep vessel step");

	return domain::matter::Vessel{domain::id::Vessel{sqlite_last_insert_id(db)}, std::move(token)};
}


domain::matter::Embodiment SqliteTemporality::embody(const domain::id::Soul soul, const domain::id::Vessel vessel)
{
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();

	SqliteStmt stmt(db, "INSERT INTO embodiments (soul_id, vessel_id) VALUES (?, ?);", "prepare embody");
	stmt.bind_i64(1, static_cast<std::int64_t>(soul.value()), "bind soul_id");
	stmt.bind_i64(2, static_cast<std::int64_t>(vessel.value()), "bind vessel_id");
	stmt.step_done("embody step");

	return domain::matter::Embodiment{soul, vessel};
}


std::vector<domain::matter::Vessel> SqliteTemporality::vessels() const
{
	std::lock_guard lock(time_db_.mutex());

	sqlite3* const db = time_db_.db();
	SqliteStmt stmt(db, "SELECT id, device_token FROM vessels ORDER BY id;", "prepare vessels");

	std::vector<domain::matter::Vessel> rows;
	while (stmt.step_row("vessels step")) {
		const domain::id::Vessel id{static_cast<std::uint64_t>(stmt.column_i64(0))};

		const auto token = domain::DeviceToken::parse(stmt.column_text(1));
		if (!token)
			throw std::runtime_error("vessels: invalid device_token in database");

		rows.push_back(domain::matter::Vessel{id, *token});
	}

	return rows;
}


std::vector<domain::matter::Embodiment> SqliteTemporality::embodiments() const
{
	std::lock_guard lock(time_db_.mutex());

	sqlite3* const db = time_db_.db();
	SqliteStmt stmt(db, "SELECT soul_id, vessel_id FROM embodiments ORDER BY soul_id;",
					"prepare embodiments");

	std::vector<domain::matter::Embodiment> rows;
	while (stmt.step_row("embodiments step")) {
		rows.push_back(domain::matter::Embodiment{
			domain::id::Soul{static_cast<std::uint64_t>(stmt.column_i64(0))},
			domain::id::Vessel{static_cast<std::uint64_t>(stmt.column_i64(1))},
		});
	}

	return rows;
}


domain::matter::Dating SqliteTemporality::date(const domain::id::Word id)
{
	const domain::Timestamp at = eternity_.time().instant();
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();
	SqliteStmt stmt(db, "INSERT OR REPLACE INTO datings (word_id, created_at_ns) VALUES (?, ?);",
					"prepare date");
	stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind word_id");
	stmt.bind_i64(2, at.value(), "bind created_at");
	stmt.step_done("date step");

	return domain::matter::Dating{id, at};
}


std::vector<domain::matter::Dating> SqliteTemporality::datings(const std::vector<domain::id::Word>& ids) const
{
	std::vector<domain::matter::Dating> out;
	out.reserve(ids.size());
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();
	for (const domain::id::Word id : ids) {
		SqliteStmt stmt(db, "SELECT word_id, created_at_ns FROM datings WHERE word_id = ?;",
						"prepare dating");
		stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind id");
		if (stmt.step_row("dating step")) {
			out.push_back(domain::matter::Dating{
				domain::id::Word{static_cast<std::uint64_t>(stmt.column_i64(0))},
				domain::Timestamp{stmt.column_i64(1)},
			});
		}
	}
	return out;
}


domain::matter::Supplication SqliteTemporality::ask(const domain::id::Soul suppliant,
									  const domain::id::Soul addressee)
{
	const domain::matter::Supplication kept{suppliant, addressee};
	const domain::Timestamp at = eternity_.time().instant();
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();

	if (awaiting_supplication(db, suppliant, addressee))
		throw std::logic_error("pending supplication already exists for this pair");

	SqliteStmt stmt(db,
					"INSERT INTO supplications (suppliant_soul_id, addressee_soul_id, asked_at_ns) "
					"VALUES (?, ?, ?);",
					"prepare ask");
	stmt.bind_i64(1, static_cast<std::int64_t>(suppliant.value()), "bind suppliant");
	stmt.bind_i64(2, static_cast<std::int64_t>(addressee.value()), "bind addressee");
	stmt.bind_i64(3, at.value(), "bind asked_at");
	stmt.step_done("ask step");

	return kept;
}


std::vector<domain::matter::Supplication> SqliteTemporality::supplications(const domain::id::Soul addressee) const
{
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();
	SqliteStmt stmt(db,
					"SELECT a.suppliant_soul_id, a.addressee_soul_id FROM supplications AS a "
					"WHERE a.addressee_soul_id = ? "
					"AND NOT EXISTS (SELECT 1 FROM answers AS r WHERE r.supplication_id = a.id) "
					"ORDER BY a.id;",
					"prepare supplications");
	stmt.bind_i64(1, static_cast<std::int64_t>(addressee.value()), "bind addressee");

	std::vector<domain::matter::Supplication> rows;
	while (stmt.step_row("supplications step")) {
		rows.push_back(domain::matter::Supplication{
			domain::id::Soul{static_cast<std::uint64_t>(stmt.column_i64(0))},
			domain::id::Soul{static_cast<std::uint64_t>(stmt.column_i64(1))},
		});
	}

	return rows;
}


domain::matter::Answer SqliteTemporality::answer(const domain::id::Soul suppliant, const domain::id::Soul addressee,
												const domain::matter::Answer::Form form)
{
	const domain::matter::Answer kept{suppliant, addressee, form};
	const domain::Timestamp at = eternity_.time().instant();
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();

	const std::optional<std::int64_t> awaiting = awaiting_supplication(db, suppliant, addressee);
	if (!awaiting)
		throw std::invalid_argument("unknown supplication");

	SqliteStmt stmt(db, "INSERT INTO answers (supplication_id, accepted, answered_at_ns) VALUES (?, ?, ?);",
					"prepare answer");
	stmt.bind_i64(1, *awaiting, "bind supplication_id");
	stmt.bind_i64(2, form == domain::matter::Answer::Form::Accepted ? 1 : 0, "bind accepted");
	stmt.bind_i64(3, at.value(), "bind answered_at");
	stmt.step_done("answer step");

	return kept;
}


domain::matter::Execution SqliteTemporality::execute(const domain::id::Word deed, const domain::id::Word behest)
{
	const domain::matter::Execution execution{deed, behest};

	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();

	{
		SqliteStmt kept(db, "SELECT 1 FROM executions WHERE behest_word_id = ?;", "prepare kept execution");
		kept.bind_i64(1, static_cast<std::int64_t>(behest.value()), "bind behest_word_id");
		if (kept.step_row("kept execution step"))
			throw std::logic_error("behest is already executed");
	}

	SqliteStmt stmt(db, "INSERT INTO executions (deed_word_id, behest_word_id) VALUES (?, ?);",
					"prepare execute");
	stmt.bind_i64(1, static_cast<std::int64_t>(deed.value()), "bind deed_word_id");
	stmt.bind_i64(2, static_cast<std::int64_t>(behest.value()), "bind behest_word_id");
	stmt.step_done("execute step");

	return execution;
}


std::vector<domain::matter::Execution>
SqliteTemporality::executions(const std::vector<domain::id::Word>& words) const
{
	std::vector<domain::matter::Execution> out;
	std::lock_guard lock(time_db_.mutex());
	sqlite3* const db = time_db_.db();
	for (const domain::id::Word word : words) {
		SqliteStmt stmt(db,
						"SELECT deed_word_id, behest_word_id FROM executions "
						"WHERE deed_word_id = ? OR behest_word_id = ?;",
						"prepare execution");
		stmt.bind_i64(1, static_cast<std::int64_t>(word.value()), "bind word");
		stmt.bind_i64(2, static_cast<std::int64_t>(word.value()), "bind word");
		while (stmt.step_row("execution step")) {
			domain::matter::Execution row{
				domain::id::Word{static_cast<std::uint64_t>(stmt.column_i64(0))},
				domain::id::Word{static_cast<std::uint64_t>(stmt.column_i64(1))},
			};
			bool seen = false;
			for (const domain::matter::Execution& kept : out)
				seen = seen || kept.id() == row.id();
			if (!seen)
				out.push_back(std::move(row));
		}
	}
	return out;
}


} // namespace will
