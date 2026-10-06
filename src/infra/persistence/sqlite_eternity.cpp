#include "sqlite_eternity.h"

#include "sqlite_spatiality.h"
#include "sqlite_temporality.h"
#include "sqlite_util.h"
#include "values/saying.h"

#include <stdexcept>
#include <string>
#include <sqlite3.h>


namespace will {


namespace {


std::string face_path(std::string prefix, const char* face)
{
	if (prefix.size() > 3 && prefix.ends_with(".db"))
		prefix.resize(prefix.size() - 3);
	return prefix + "." + face + ".db";
}


} // namespace


SqliteEternity::SqliteEternity(std::string prefix)
	: prefix_(std::move(prefix))
	, database_(face_path(prefix_, "eternity"), SqliteFace::Eternity)
	, space_(database_)
{}


std::unique_ptr<domain::Spatiality> SqliteEternity::spatiality(domain::Birth<domain::Creation>)
{
	return std::make_unique<SqliteSpatiality>(face_path(prefix_, "space"), *this);
}


std::unique_ptr<domain::Temporality> SqliteEternity::temporality(domain::Birth<domain::Creation>)
{
	return std::make_unique<SqliteTemporality>(face_path(prefix_, "time"), *this);
}


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


domain::matter::Fatherhood SqliteEternity::father(const domain::id::Soul father, const domain::id::Soul child,
												  const domain::matter::Fatherhood::Line line)
{
	const domain::matter::Fatherhood kept{father, child, line};

	std::lock_guard lock(database_.mutex());
	sqlite3* const db = database_.db();

	if (line == domain::matter::Fatherhood::Line::Flesh) {
		SqliteStmt had(db, "SELECT 1 FROM fatherhoods WHERE child_soul_id = ? AND line = ?;",
					   "prepare fatherhood by flesh");
		had.bind_i64(1, static_cast<std::int64_t>(child.value()), "bind child");
		had.bind_i64(2, static_cast<std::int64_t>(line), "bind line");
		if (had.step_row("fatherhood by flesh step"))
			throw std::logic_error("the child already has his father by flesh");
	}

	SqliteStmt stmt(db, "INSERT OR REPLACE INTO fatherhoods (child_soul_id, line, father_soul_id) VALUES (?, ?, ?);",
					"prepare father");
	stmt.bind_i64(1, static_cast<std::int64_t>(child.value()), "bind child");
	stmt.bind_i64(2, static_cast<std::int64_t>(line), "bind line");
	stmt.bind_i64(3, static_cast<std::int64_t>(father.value()), "bind father");
	stmt.step_done("father step");

	return kept;
}


std::vector<domain::matter::Fatherhood> SqliteEternity::fatherhoods() const
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db, "SELECT father_soul_id, child_soul_id, line FROM fatherhoods ORDER BY child_soul_id, line;",
					"prepare fatherhoods");

	std::vector<domain::matter::Fatherhood> rows;
	while (stmt.step_row("fatherhoods step")) {
		const std::int64_t line = stmt.column_i64(2);
		if (line < 0 || line > static_cast<std::int64_t>(domain::matter::Fatherhood::Line::Spirit))
			throw std::runtime_error("fatherhoods: unknown line in database");

		rows.push_back(domain::matter::Fatherhood{
			domain::id::Soul{static_cast<std::uint64_t>(stmt.column_i64(0))},
			domain::id::Soul{static_cast<std::uint64_t>(stmt.column_i64(1))},
			static_cast<domain::matter::Fatherhood::Line>(line),
		});
	}

	return rows;
}


domain::matter::Word SqliteEternity::utter(const domain::id::Soul author, const domain::Saying& saying)
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db, "INSERT INTO words (author_soul_id, body) VALUES (?, ?);",
					"prepare insert word");
	stmt.bind_i64(1, static_cast<std::int64_t>(author.value()), "bind author");
	stmt.bind_text(2, saying.body(), "bind body");
	stmt.step_done("insert word step");

	return domain::matter::Word{domain::id::Word{sqlite_last_insert_id(db)}, author, saying};
}


domain::matter::Word SqliteEternity::word(const domain::id::Word id) const
{
	std::lock_guard lock(database_.mutex());

	sqlite3* const db = database_.db();
	SqliteStmt stmt(db, "SELECT id, author_soul_id, body FROM words WHERE id = ?;",
					"prepare word");
	stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind id");
	if (!stmt.step_row("word step"))
		throw std::invalid_argument("unknown word");

	return domain::matter::Word{
		domain::id::Word{static_cast<std::uint64_t>(stmt.column_i64(0))},
		domain::id::Soul{static_cast<std::uint64_t>(stmt.column_i64(1))},
		std::string(stmt.column_text(2)),
	};
}


std::vector<domain::matter::Word> SqliteEternity::words(const std::vector<domain::id::Word>& ids) const
{
	std::vector<domain::matter::Word> out;
	out.reserve(ids.size());
	for (const domain::id::Word id : ids) {
		try {
			out.push_back(word(id));
		} catch (const std::invalid_argument&) {
		}
	}
	return out;
}


} // namespace will
