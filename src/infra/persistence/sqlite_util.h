#pragma once

#include <cstdint>
#include <format>
#include <sqlite3.h>
#include <stdexcept>
#include <string_view>


namespace will {


inline void check_sqlite(const int rc, sqlite3* db, const char* context)
{
	if (rc == SQLITE_OK || rc == SQLITE_DONE || rc == SQLITE_ROW)
		return;

	const char* message = db ? sqlite3_errmsg(db) : sqlite3_errstr(rc);
	throw std::runtime_error(std::format("{}: {}", context, message ? message : "unknown error"));
}


inline std::uint64_t sqlite_last_insert_id(sqlite3* db)
{
	return static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db));
}


/// RAII SQLite prepared statement.
class SqliteStmt {
public:
	SqliteStmt(sqlite3* db, const char* sql, const char* context)
		: db_(db)
	{
		check_sqlite(sqlite3_prepare_v2(db_, sql, -1, &stmt_, nullptr), db_, context);
	}

	SqliteStmt(const SqliteStmt&) = delete;
	SqliteStmt& operator=(const SqliteStmt&) = delete;

	~SqliteStmt()
	{
		if (stmt_)
			sqlite3_finalize(stmt_);
	}

	void bind_i64(const int i, const std::int64_t v, const char* context)
	{
		check_sqlite(sqlite3_bind_int64(stmt_, i, v), db_, context);
	}

	void bind_text(const int i, const std::string_view v, const char* context)
	{
		check_sqlite(sqlite3_bind_text(stmt_, i, v.data(), static_cast<int>(v.size()), SQLITE_TRANSIENT),
					 db_, context);
	}

	/// Step once; returns the raw result code after check_sqlite (ROW / DONE / OK).
	int step(const char* context)
	{
		const int rc = sqlite3_step(stmt_);
		check_sqlite(rc, db_, context);
		return rc;
	}

	/// True on SQLITE_ROW; false on SQLITE_DONE; throws on error.
	bool step_row(const char* context)
	{
		const int rc = sqlite3_step(stmt_);
		if (rc == SQLITE_ROW)
			return true;
		check_sqlite(rc, db_, context);
		return false;
	}

	/// Expect SQLITE_DONE (INSERT / UPDATE / DELETE).
	void step_done(const char* context)
	{
		const int rc = sqlite3_step(stmt_);
		if (rc == SQLITE_DONE)
			return;
		check_sqlite(rc, db_, context);
		throw std::runtime_error(std::format("{}: expected SQLITE_DONE", context));
	}

	std::int64_t column_i64(const int i) const
	{
		return sqlite3_column_int64(stmt_, i);
	}

	std::string_view column_text(const int i) const
	{
		const unsigned char* const text = sqlite3_column_text(stmt_, i);
		if (!text)
			return {};
		return {reinterpret_cast<const char*>(text),
				static_cast<std::size_t>(sqlite3_column_bytes(stmt_, i))};
	}

	bool column_is_null(const int i) const
	{
		return sqlite3_column_type(stmt_, i) == SQLITE_NULL;
	}

	sqlite3_stmt* get() const { return stmt_; }

private:
	sqlite3* db_ = nullptr;
	sqlite3_stmt* stmt_ = nullptr;
};


/// RAII SQLite transaction: ROLLBACK on destruction unless commit() succeeded.
class SqliteTransaction {
public:
	explicit SqliteTransaction(sqlite3* db)
		: db_(db)
	{
		check_sqlite(sqlite3_exec(db_, "BEGIN IMMEDIATE;", nullptr, nullptr, nullptr), db_,
					 "begin transaction");
	}

	SqliteTransaction(const SqliteTransaction&) = delete;
	SqliteTransaction& operator=(const SqliteTransaction&) = delete;

	~SqliteTransaction()
	{
		if (db_ && !committed_)
			sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
	}

	void commit()
	{
		check_sqlite(sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, nullptr), db_, "commit transaction");
		committed_ = true;
	}

private:
	sqlite3* db_ = nullptr;
	bool committed_ = false;
};


} // namespace will
