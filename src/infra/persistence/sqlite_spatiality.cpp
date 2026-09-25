#include "sqlite_spatiality.h"

#include "sqlite_util.h"
#include "values/abode_name.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <sqlite3.h>


namespace will {


SqliteSpatiality::SqliteSpatiality(SqliteDatabase& database)
	: database_(database)
{}


std::vector<domain::Abode> SqliteSpatiality::abodes()
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db, "SELECT id, name FROM abodes ORDER BY id;", "prepare abodes");

	std::vector<domain::Abode> rows;
	while (stmt.step_row("abodes step")) {
		const domain::id::Abode id{static_cast<std::uint64_t>(stmt.column_i64(0))};
		const std::string_view name_text = stmt.column_text(1);
		if (name_text.empty())
			throw std::runtime_error("abodes: missing name in database");

		rows.emplace_back(id, domain::AbodeName{name_text});
	}

	return rows;
}


std::optional<domain::Abode> SqliteSpatiality::abode_of(const domain::id::Soul soul) const
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

	return domain::Abode{id, domain::AbodeName{name_text}};
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


void SqliteSpatiality::place(const domain::id::Letter id, const domain::id::Place place)
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db, "INSERT OR REPLACE INTO placements (letter_id, place_id) VALUES (?, ?);",
					"prepare place");
	stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind letter_id");
	stmt.bind_i64(2, static_cast<std::int64_t>(place.value()), "bind place_id");
	stmt.step_done("place step");
}


std::vector<domain::Placement> SqliteSpatiality::placements(const domain::id::Place place,
															const std::uint32_t limit) const
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db,
					"SELECT letter_id, place_id FROM placements "
					"WHERE place_id = ? ORDER BY letter_id DESC LIMIT ?;",
					"prepare placements");
	stmt.bind_i64(1, static_cast<std::int64_t>(place.value()), "bind place_id");
	stmt.bind_i64(2, static_cast<std::int64_t>(limit), "bind limit");

	std::vector<domain::Placement> rows;
	rows.reserve(limit);
	while (stmt.step_row("placements step")) {
		rows.push_back(domain::Placement{
			domain::id::Letter{static_cast<std::uint64_t>(stmt.column_i64(0))},
			domain::id::Place{static_cast<std::uint64_t>(stmt.column_i64(1))},
		});
	}
	return rows;
}


} // namespace will
