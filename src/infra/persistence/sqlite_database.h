#pragma once

#include <mutex>
#include <string>


struct sqlite3;


namespace will {


/** Owns the SQLite connection and schema. */
class SqliteDatabase {
public:
	explicit SqliteDatabase(std::string db_path);
	~SqliteDatabase();

	SqliteDatabase(const SqliteDatabase&) = delete;
	SqliteDatabase& operator=(const SqliteDatabase&) = delete;

	[[nodiscard]] sqlite3* db() const noexcept { return db_; }
	[[nodiscard]] std::mutex& mutex() noexcept { return mutex_; }

private:
	void open_database();
	void init_schema();

	std::string db_path_;
	sqlite3* db_ = nullptr;
	std::mutex mutex_;
};


} // namespace will
