#include "sqlite_spatiality.h"

#include "beings/space.h"
#include "sqlite_util.h"
#include "values/abode_name.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <sqlite3.h>


namespace will {


SqliteSpatiality::SqliteSpatiality(SqliteDatabase& database, domain::Eternity& eternity)
	: database_(database)
	, eternity_(eternity)
{}


std::vector<domain::matter::Abode> SqliteSpatiality::abodes()
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db, "SELECT id, name FROM abodes ORDER BY id;", "prepare abodes");

	std::vector<domain::matter::Abode> rows;
	while (stmt.step_row("abodes step")) {
		const domain::id::Abode id{static_cast<std::uint64_t>(stmt.column_i64(0))};
		const std::string_view name_text = stmt.column_text(1);
		if (name_text.empty())
			throw std::runtime_error("abodes: missing name in database");

		rows.emplace_back(id, domain::AbodeName{name_text});
	}

	return rows;
}


domain::matter::Abode SqliteSpatiality::abide(const domain::id::Soul soul, domain::AbodeName name)
{
	if (std::optional<domain::matter::Abode> existing = abode_of(soul))
		return std::move(*existing);

	const domain::id::Abode id{eternity_.space().point()};
	keep(id, name);
	join_abode(id, soul);
	return domain::matter::Abode{id, std::move(name)};
}


std::optional<domain::matter::Abode> SqliteSpatiality::abode_of(const domain::id::Soul soul) const
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db,
					"SELECT a.id, a.name FROM abodes a "
					"INNER JOIN abode_souls s ON s.abode_id = a.id "
					"WHERE s.soul_id = ? ORDER BY a.id LIMIT 1;",
					"prepare abode_of");
	stmt.bind_i64(1, static_cast<std::int64_t>(soul.value()), "bind soul_id");

	if (!stmt.step_row("abode_of step"))
		return std::nullopt;

	const domain::id::Abode id{static_cast<std::uint64_t>(stmt.column_i64(0))};
	const std::string_view name_text = stmt.column_text(1);
	if (name_text.empty())
		throw std::runtime_error("abode_of: missing name in database");

	return domain::matter::Abode{id, domain::AbodeName{name_text}};
}


void SqliteSpatiality::keep(const domain::id::Abode id, domain::AbodeName name)
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db, "INSERT OR IGNORE INTO abodes (id, name) VALUES (?, ?);", "prepare keep abode");
	stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind abode id");
	stmt.bind_text(2, name.text(), "bind abode name");
	stmt.step_done("keep abode step");
}


void SqliteSpatiality::join_abode(const domain::id::Abode abode, const domain::id::Soul soul)
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db, "INSERT OR IGNORE INTO abode_souls (abode_id, soul_id) VALUES (?, ?);",
					"prepare join abode_souls");
	stmt.bind_i64(1, static_cast<std::int64_t>(abode.value()), "bind abode_id");
	stmt.bind_i64(2, static_cast<std::int64_t>(soul.value()), "bind soul_id");
	stmt.step_done("insert abode_souls step");
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


void SqliteSpatiality::place(const domain::id::Word id, const domain::id::Place place)
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db, "INSERT OR REPLACE INTO placements (word_id, place_id) VALUES (?, ?);",
					"prepare place");
	stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind word_id");
	stmt.bind_i64(2, static_cast<std::int64_t>(place.value()), "bind place_id");
	stmt.step_done("place step");
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
