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

	const domain::id::Place id{static_cast<std::uint64_t>(stmt.column_i64(0))};
	const std::string_view name_text = stmt.column_text(1);
	if (name_text.empty())
		throw std::runtime_error("abode: missing name in database");

	return domain::matter::Abode{id, domain::AbodeName{name_text}};
}


domain::matter::Room::Aspect aspect_of(const std::int64_t kept)
{
	if (kept < 0 || kept > static_cast<std::int64_t>(domain::matter::Room::Aspect::Dwellers))
		throw std::runtime_error("room: unknown aspect in database");
	return static_cast<domain::matter::Room::Aspect>(kept);
}


} // namespace


SqliteSpatiality::SqliteSpatiality(std::string path, domain::Eternity& eternity)
	: database_(std::move(path), SqliteFace::Spatiality)
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

	const domain::id::Place id = eternity_.space().point();

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


domain::matter::Dweller SqliteSpatiality::dwell(const domain::id::Place abode, const domain::id::Soul soul,
												const domain::matter::Dweller::Kind kind)
{
	const domain::matter::Dweller kept{abode, soul, kind};
	std::lock_guard lock(database_.mutex());

	SqliteStmt stmt(database_.db(), "INSERT OR REPLACE INTO dwellers (abode_id, soul_id, kind) VALUES (?, ?, ?);",
					"prepare dwell");
	stmt.bind_i64(1, static_cast<std::int64_t>(abode.value()), "bind abode_id");
	stmt.bind_i64(2, static_cast<std::int64_t>(soul.value()), "bind soul_id");
	stmt.bind_i64(3, static_cast<std::int64_t>(kind), "bind kind");
	stmt.step_done("dwell step");

	return kept;
}


std::vector<domain::matter::Dweller> SqliteSpatiality::dwellers() const
{
	std::lock_guard lock(database_.mutex());

	SqliteStmt stmt(database_.db(), "SELECT abode_id, soul_id, kind FROM dwellers ORDER BY abode_id, soul_id;",
					"prepare dwellers");

	std::vector<domain::matter::Dweller> rows;
	while (stmt.step_row("dwellers step")) {
		const std::int64_t kind = stmt.column_i64(2);
		if (kind < 0 || kind > static_cast<std::int64_t>(domain::matter::Dweller::Kind::Friend))
			throw std::runtime_error("dweller: unknown kind in database");

		rows.emplace_back(domain::id::Place{static_cast<std::uint64_t>(stmt.column_i64(0))},
						  domain::id::Soul{static_cast<std::uint64_t>(stmt.column_i64(1))},
						  static_cast<domain::matter::Dweller::Kind>(kind));
	}

	return rows;
}


domain::matter::Room SqliteSpatiality::furnish(const domain::id::Place abode, const domain::id::Place reflects,
											   const domain::matter::Room::Aspect aspect)
{
	std::lock_guard lock(database_.mutex());
	sqlite3* const db = database_.db();

	{
		SqliteStmt kept(db, "SELECT 1 FROM rooms WHERE abode_id = ? AND place_id = ? AND aspect = ?;",
						"prepare kept room");
		kept.bind_i64(1, static_cast<std::int64_t>(abode.value()), "bind abode_id");
		kept.bind_i64(2, static_cast<std::int64_t>(reflects.value()), "bind place_id");
		kept.bind_i64(3, static_cast<std::int64_t>(aspect), "bind aspect");
		if (kept.step_row("kept room step"))
			throw std::logic_error("the abode already has a room reflecting this place");
	}

	const domain::matter::Room room{eternity_.space().point(), abode, reflects, aspect,
									domain::matter::Room::Part::Inner};

	SqliteStmt stmt(db, "INSERT INTO rooms (id, abode_id, place_id, aspect, part) VALUES (?, ?, ?, ?, ?);",
					"prepare furnish");
	stmt.bind_i64(1, static_cast<std::int64_t>(room.id().value()), "bind id");
	stmt.bind_i64(2, static_cast<std::int64_t>(abode.value()), "bind abode_id");
	stmt.bind_i64(3, static_cast<std::int64_t>(reflects.value()), "bind place_id");
	stmt.bind_i64(4, static_cast<std::int64_t>(aspect), "bind aspect");
	stmt.bind_i64(5, static_cast<std::int64_t>(room.part()), "bind part");
	stmt.step_done("furnish step");

	return room;
}


domain::matter::Room SqliteSpatiality::arrange(const domain::id::Place room, const domain::matter::Room::Part part)
{
	std::lock_guard lock(database_.mutex());
	sqlite3* const db = database_.db();

	SqliteStmt kept(db, "SELECT abode_id, place_id, aspect FROM rooms WHERE id = ?;", "prepare arranged room");
	kept.bind_i64(1, static_cast<std::int64_t>(room.value()), "bind id");
	if (!kept.step_row("arranged room step"))
		throw std::invalid_argument("unknown room");
	const domain::matter::Room arranged{room, domain::id::Place{static_cast<std::uint64_t>(kept.column_i64(0))},
										domain::id::Place{static_cast<std::uint64_t>(kept.column_i64(1))},
										aspect_of(kept.column_i64(2)), part};

	SqliteStmt stmt(db, "UPDATE rooms SET part = ? WHERE id = ?;", "prepare arrange");
	stmt.bind_i64(1, static_cast<std::int64_t>(part), "bind part");
	stmt.bind_i64(2, static_cast<std::int64_t>(room.value()), "bind id");
	stmt.step_done("arrange step");

	return arranged;
}


std::vector<domain::matter::Room> SqliteSpatiality::rooms(const domain::id::Place abode) const
{
	std::lock_guard lock(database_.mutex());

	SqliteStmt stmt(database_.db(), "SELECT id, place_id, part, aspect FROM rooms WHERE abode_id = ? ORDER BY id;",
					"prepare rooms");
	stmt.bind_i64(1, static_cast<std::int64_t>(abode.value()), "bind abode_id");

	std::vector<domain::matter::Room> rows;
	while (stmt.step_row("rooms step")) {
		const std::int64_t part = stmt.column_i64(2);
		if (part < 0 || part > static_cast<std::int64_t>(domain::matter::Room::Part::Outer))
			throw std::runtime_error("room: unknown part in database");

		rows.emplace_back(domain::id::Place{static_cast<std::uint64_t>(stmt.column_i64(0))}, abode,
						  domain::id::Place{static_cast<std::uint64_t>(stmt.column_i64(1))},
						  aspect_of(stmt.column_i64(3)), static_cast<domain::matter::Room::Part>(part));
	}

	return rows;
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

	const domain::matter::Tie kept{eternity_.space().point(), testator, novice};

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
			domain::id::Place{static_cast<std::uint64_t>(stmt.column_i64(0))},
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
