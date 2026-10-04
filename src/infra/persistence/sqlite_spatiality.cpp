#include "sqlite_spatiality.h"

#include "horizons/space.h"
#include "sqlite_util.h"
#include "values/abode_name.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <sqlite3.h>


namespace will {
namespace {


std::optional<domain::matter::Abode> kept_abode(sqlite3* db, const domain::id::Soul host)
{
	SqliteStmt stmt(db,
					"SELECT a.id, a.name FROM abodes a "
					"INNER JOIN abode_souls s ON s.abode_id = a.id "
					"WHERE s.soul_id = ? ORDER BY a.id LIMIT 1;",
					"prepare abode");
	stmt.bind_i64(1, static_cast<std::int64_t>(host.value()), "bind soul_id");

	if (!stmt.step_row("abode step"))
		return std::nullopt;

	const domain::id::Abode id{static_cast<std::uint64_t>(stmt.column_i64(0))};
	const std::string_view name_text = stmt.column_text(1);
	if (name_text.empty())
		throw std::runtime_error("abode: missing name in database");

	return domain::matter::Abode{id, domain::AbodeName{name_text}};
}


} // namespace


SqliteSpatiality::SqliteSpatiality(SqliteDatabase& database, domain::Eternity& eternity)
	: database_(database)
	, eternity_(eternity)
{}


std::optional<domain::matter::Abode> SqliteSpatiality::abode(const domain::id::Soul host) const
{
	std::lock_guard lock(database_.mutex());
	return kept_abode(database_.db(), host);
}


domain::matter::Abode SqliteSpatiality::abide(const domain::id::Soul host, domain::AbodeName name)
{
	std::lock_guard lock(database_.mutex());
	sqlite3* const db = database_.db();
	SqliteTransaction tx(db);

	if (kept_abode(db, host))
		throw std::logic_error("soul already keeps an abode");

	const domain::id::Abode id{eternity_.space().point()};

	SqliteStmt abode_stmt(db, "INSERT INTO abodes (id, name) VALUES (?, ?);", "prepare abide");
	abode_stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind abode id");
	abode_stmt.bind_text(2, name.text(), "bind abode name");
	abode_stmt.step_done("abide step");

	SqliteStmt soul_stmt(db, "INSERT INTO abode_souls (abode_id, soul_id) VALUES (?, ?);",
						 "prepare abide soul");
	soul_stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind abode_id");
	soul_stmt.bind_i64(2, static_cast<std::int64_t>(host.value()), "bind soul_id");
	soul_stmt.step_done("abide soul step");

	tx.commit();

	return domain::matter::Abode{id, std::move(name)};
}


domain::matter::Tie SqliteSpatiality::bind(const domain::id::Soul testator, const domain::id::Soul novice)
{
	std::lock_guard lock(database_.mutex());
	sqlite3* const db = database_.db();

	{
		SqliteStmt bound(db,
						 "SELECT 1 FROM ties "
						 "WHERE testator_soul_id = ? AND novice_soul_id = ? LIMIT 1;",
						 "prepare bound pair");
		bound.bind_i64(1, static_cast<std::int64_t>(testator.value()), "bind testator");
		bound.bind_i64(2, static_cast<std::int64_t>(novice.value()), "bind novice");
		if (bound.step_row("bound pair step"))
			throw std::logic_error("obedience already exists for this pair");
	}

	const domain::matter::Tie kept{domain::id::Tie{eternity_.space().point()}, testator, novice};

	SqliteStmt stmt(db,
					"INSERT INTO ties (id, testator_soul_id, novice_soul_id) VALUES (?, ?, ?);",
					"prepare bind");
	stmt.bind_i64(1, static_cast<std::int64_t>(kept.id().value()), "bind id");
	stmt.bind_i64(2, static_cast<std::int64_t>(testator.value()), "bind testator");
	stmt.bind_i64(3, static_cast<std::int64_t>(novice.value()), "bind novice");
	stmt.step_done("bind step");

	return kept;
}


std::vector<domain::matter::Tie> SqliteSpatiality::ties() const
{
	std::lock_guard lock(database_.mutex());
	sqlite3* const db = database_.db();
	SqliteStmt stmt(db,
					"SELECT id, testator_soul_id, novice_soul_id FROM ties ORDER BY id;",
					"prepare ties");

	std::vector<domain::matter::Tie> rows;
	while (stmt.step_row("ties step")) {
		rows.push_back(domain::matter::Tie{
			domain::id::Tie{static_cast<std::uint64_t>(stmt.column_i64(0))},
			domain::id::Soul{static_cast<std::uint64_t>(stmt.column_i64(1))},
			domain::id::Soul{static_cast<std::uint64_t>(stmt.column_i64(2))},
		});
	}

	return rows;
}


domain::matter::Placement SqliteSpatiality::place(const domain::id::Word id, const domain::id::Place place)
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db, "INSERT OR REPLACE INTO placements (word_id, place_id) VALUES (?, ?);",
					"prepare place");
	stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind word_id");
	stmt.bind_i64(2, static_cast<std::int64_t>(place.value()), "bind place_id");
	stmt.step_done("place step");

	return domain::matter::Placement{id, place};
}


std::optional<domain::matter::Placement> SqliteSpatiality::placement(const domain::id::Word id) const
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db, "SELECT word_id, place_id FROM placements WHERE word_id = ?;",
					"prepare placement");
	stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind word_id");
	if (!stmt.step_row("placement step"))
		return std::nullopt;

	return domain::matter::Placement{
		domain::id::Word{static_cast<std::uint64_t>(stmt.column_i64(0))},
		domain::id::Place{static_cast<std::uint64_t>(stmt.column_i64(1))},
	};
}


std::vector<domain::matter::Placement> SqliteSpatiality::placements(const domain::id::Place place,
															const std::uint32_t limit) const
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db,
					"SELECT word_id, place_id FROM placements "
					"WHERE place_id = ? ORDER BY word_id DESC LIMIT ?;",
					"prepare placements");
	stmt.bind_i64(1, static_cast<std::int64_t>(place.value()), "bind place_id");
	stmt.bind_i64(2, static_cast<std::int64_t>(limit), "bind limit");

	std::vector<domain::matter::Placement> rows;
	rows.reserve(limit);
	while (stmt.step_row("placements step")) {
		rows.push_back(domain::matter::Placement{
			domain::id::Word{static_cast<std::uint64_t>(stmt.column_i64(0))},
			domain::id::Place{static_cast<std::uint64_t>(stmt.column_i64(1))},
		});
	}
	return rows;
}


} // namespace will
