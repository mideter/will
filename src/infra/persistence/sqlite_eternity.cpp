#include "sqlite_eternity.h"

#include "sqlite_util.h"
#include "values/saying.h"

#include <stdexcept>
#include <string>
#include <sqlite3.h>


namespace will {


SqliteEternity::SqliteEternity(SqliteDatabase& database)
	: database_(database)
	, space_(database)
{}


domain::Time& SqliteEternity::time()
{
	return time_;
}


domain::Space& SqliteEternity::space()
{
	return space_;
}


domain::matter::Soul SqliteEternity::enroll(domain::SoulName name)
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db, "INSERT INTO souls (name) VALUES (?);", "prepare enroll");
	stmt.bind_text(1, name.text(), "bind name");
	stmt.step_done("enroll step");

	return domain::matter::Soul{domain::id::Soul{sqlite_last_insert_id(db)}, std::move(name)};
}


std::vector<domain::matter::Soul> SqliteEternity::souls() const
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db, "SELECT id, name FROM souls ORDER BY id;", "prepare souls");

	std::vector<domain::matter::Soul> rows;
	while (stmt.step_row("souls step")) {
		const domain::id::Soul id{static_cast<std::uint64_t>(stmt.column_i64(0))};

		const auto name = domain::SoulName::parse(stmt.column_text(1));
		if (!name)
			throw std::runtime_error("souls: invalid soul name in database");

		rows.push_back(domain::matter::Soul{id, *name});
	}

	return rows;
}


domain::matter::Utterance SqliteEternity::utter(const domain::id::Soul author, const domain::Saying& saying)
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db, "INSERT INTO utterances (author_soul_id, body) VALUES (?, ?);",
					"prepare insert utterance");
	stmt.bind_i64(1, static_cast<std::int64_t>(author.value()), "bind author");
	stmt.bind_text(2, saying.body(), "bind body");
	stmt.step_done("insert utterance step");

	return domain::matter::Utterance{domain::id::Word{sqlite_last_insert_id(db)}, author, saying};
}


domain::matter::Utterance SqliteEternity::utterance(const domain::id::Word id) const
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db, "SELECT id, author_soul_id, body FROM utterances WHERE id = ?;",
					"prepare utterance");
	stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind id");
	if (!stmt.step_row("utterance step"))
		throw std::invalid_argument("unknown utterance");

	return domain::matter::Utterance{
		domain::id::Word{static_cast<std::uint64_t>(stmt.column_i64(0))},
		domain::id::Soul{static_cast<std::uint64_t>(stmt.column_i64(1))},
		std::string(stmt.column_text(2)),
	};
}


std::vector<domain::matter::Utterance> SqliteEternity::utterances(const std::vector<domain::id::Word>& ids) const
{
	std::vector<domain::matter::Utterance> out;
	out.reserve(ids.size());
	for (const domain::id::Word id : ids) {
		try {
			out.push_back(utterance(id));
		} catch (const std::invalid_argument&) {
		}
	}
	return out;
}


} // namespace will
