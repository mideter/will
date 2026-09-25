#include "sqlite_space.h"

#include "sqlite_util.h"

#include <sqlite3.h>


namespace will {
namespace {


std::uint64_t read_hwm(sqlite3* db)
{
	SqliteStmt stmt(db, "SELECT value FROM place_hwm WHERE id = 1;", "prepare read hwm");
	if (!stmt.step_row("read hwm step"))
		return 0;
	return static_cast<std::uint64_t>(stmt.column_i64(0));
}


} // namespace


SqliteSpace::SqliteSpace(SqliteDatabase& database)
	: database_(database)
{
	std::lock_guard lock(database_.mutex());
	seed(read_hwm(database_.db()));
}


void SqliteSpace::persist_mark(const domain::id::Place id)
{
	std::lock_guard lock(database_.mutex());
	sqlite3* const db = database_.db();

	SqliteStmt stmt(db, "UPDATE place_hwm SET value = MAX(value, ?) WHERE id = 1;",
					"prepare persist place mark");
	stmt.bind_i64(1, static_cast<std::int64_t>(id.value()), "bind value");
	stmt.step_done("persist place mark step");
}


} // namespace will
