#pragma once

#include <format>
#include <sqlite3.h>
#include <stdexcept>


namespace will {


inline void check_sqlite(const int rc, sqlite3* db, const char* context)
{
	if (rc == SQLITE_OK || rc == SQLITE_DONE || rc == SQLITE_ROW)
		return;

	const char* message = db ? sqlite3_errmsg(db) : sqlite3_errstr(rc);
	throw std::runtime_error(std::format("{}: {}", context, message ? message : "unknown error"));
}


} // namespace will
