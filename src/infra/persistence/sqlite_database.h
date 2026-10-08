#pragma once

#include <mutex>
#include <string>


struct sqlite3;


namespace will {


enum class SqliteFace {
	Eternity,
	Spatiality,
	Temporality,
};


/** Owns one SQLite connection and the schema for one face of being. */
class SqliteDatabase {
public:
	SqliteDatabase(std::string db_path, SqliteFace face);
	~SqliteDatabase();

	SqliteDatabase(const SqliteDatabase&) = delete;
	SqliteDatabase& operator=(const SqliteDatabase&) = delete;

	[[nodiscard]] sqlite3* db() const noexcept { return db_; }
	[[nodiscard]] std::mutex& mutex() noexcept { return mutex_; }
	[[nodiscard]] SqliteFace face() const noexcept { return face_; }

private:
	void open_database();
	void init_schema();

	/// Rejections kept before answers were (table `rejections`) become answers.
	void answer_kept_rejections();

	/// Rooms kept before they had an aspect (all windows onto words) get one.
	void give_rooms_their_aspect();

	/// Approaches kept before they had a rest are given none.
	void give_approaches_their_rest();

	std::string db_path_;
	SqliteFace face_;
	sqlite3* db_ = nullptr;
	std::mutex mutex_;
};


} // namespace will
