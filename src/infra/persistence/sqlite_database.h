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

	void answer_kept_rejections();

	void give_rooms_their_aspect();

	void fold_birth_rooms_into_gates();

	void give_approaches_their_rest();
	void give_exercises_their_instruction();

	std::string db_path_;
	SqliteFace face_;
	sqlite3* db_ = nullptr;
	std::mutex mutex_;
};


} // namespace will
