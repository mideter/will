#include "sqlite_eternity.h"

#include "sqlite_util.h"
#include "values/word.h"

#include <stdexcept>
#include <string>
#include <sqlite3.h>


namespace will {


SqliteEternity::SqliteEternity(SqliteDatabase& database)
	: database_(database)
{}


domain::Time& SqliteEternity::time()
{
	return time_;
}


domain::id::Soul SqliteEternity::enroll(const domain::SoulName name)
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db, "INSERT INTO souls (name) VALUES (?);", "prepare insert soul");
	stmt.bind_text(1, name.text(), "bind name");
	stmt.step_done("insert soul step");

	return domain::id::Soul{sqlite_last_insert_id(db)};
}


domain::Utterance SqliteEternity::utter(const domain::id::Soul author, const domain::Word& word)
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db, "INSERT INTO utterances (author_soul_id, body) VALUES (?, ?);",
					"prepare insert utterance");
	stmt.bind_i64(1, static_cast<std::int64_t>(author.value()), "bind author");
	stmt.bind_text(2, word.body(), "bind body");
	stmt.step_done("insert utterance step");

	return domain::Utterance{domain::id::Letter{sqlite_last_insert_id(db)}, author, word};
}


domain::Utterance SqliteEternity::utterance(const domain::id::Letter id) const
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db, "SELECT id, author_soul_id, body FROM utterances WHERE id = ?;",
					"prepare utterance");
	stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind id");
	if (!stmt.step_row("utterance step"))
		throw std::invalid_argument("unknown utterance");

	return domain::Utterance{
		domain::id::Letter{static_cast<std::uint64_t>(stmt.column_i64(0))},
		domain::id::Soul{static_cast<std::uint64_t>(stmt.column_i64(1))},
		domain::Word{std::string(stmt.column_text(2))},
	};
}


std::vector<domain::Utterance> SqliteEternity::utterances(const std::vector<domain::id::Letter>& ids) const
{
	std::vector<domain::Utterance> out;
	out.reserve(ids.size());
	for (const domain::id::Letter id : ids) {
		try {
			out.push_back(utterance(id));
		} catch (const std::invalid_argument&) {
		}
	}
	return out;
}


} // namespace will
