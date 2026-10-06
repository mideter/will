#include "sqlite_database.h"

#include <string_view>

#include "sqlite_util.h"

#include <stdexcept>
#include <sqlite3.h>


namespace will {


SqliteDatabase::SqliteDatabase(std::string db_path, SqliteFace face)
	: db_path_(std::move(db_path))
	, face_(face)
{
	open_database();
	init_schema();
}


SqliteDatabase::~SqliteDatabase()
{
	if (db_)
		sqlite3_close(db_);
}


void SqliteDatabase::open_database()
{
	check_sqlite(sqlite3_open_v2(db_path_.c_str(), &db_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr),
				 db_, "sqlite3_open_v2");

	check_sqlite(sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr), db_,
				 "PRAGMA journal_mode");
	check_sqlite(sqlite3_exec(db_, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr), db_,
				 "PRAGMA synchronous");
}


void SqliteDatabase::init_schema()
{
	const char* sql = nullptr;
	switch (face_) {
	case SqliteFace::Eternity:
		sql = R"sql(
CREATE TABLE IF NOT EXISTS souls (
  id INTEGER PRIMARY KEY,
  name TEXT NOT NULL DEFAULT ''
);

CREATE TABLE IF NOT EXISTS words (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  author_soul_id INTEGER NOT NULL,
  body TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS place_hwm (
  id INTEGER PRIMARY KEY CHECK (id = 1),
  value INTEGER NOT NULL
);
INSERT OR IGNORE INTO place_hwm (id, value) VALUES (1, 0);
)sql";
		break;
	case SqliteFace::Spatiality:
		sql = R"sql(
CREATE TABLE IF NOT EXISTS abodes (
  id INTEGER PRIMARY KEY,
  name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS abode_souls (
  abode_id INTEGER NOT NULL,
  soul_id INTEGER NOT NULL,
  PRIMARY KEY (abode_id, soul_id)
);

CREATE TABLE IF NOT EXISTS dwellers (
  abode_id INTEGER NOT NULL,
  soul_id INTEGER NOT NULL,
  kind INTEGER NOT NULL,
  PRIMARY KEY (abode_id, soul_id)
);

CREATE TABLE IF NOT EXISTS rooms (
  id INTEGER PRIMARY KEY,
  abode_id INTEGER NOT NULL,
  place_id INTEGER NOT NULL,
  aspect INTEGER NOT NULL DEFAULT 0,
  part INTEGER NOT NULL,
  UNIQUE (abode_id, place_id, aspect)
);

CREATE TABLE IF NOT EXISTS placements (
  word_id INTEGER PRIMARY KEY,
  place_id INTEGER NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_placements_place ON placements(place_id);

CREATE TABLE IF NOT EXISTS ties (
  id INTEGER PRIMARY KEY,
  testator_soul_id INTEGER NOT NULL,
  novice_soul_id INTEGER NOT NULL
);
)sql";
		break;
	case SqliteFace::Temporality:
		sql = R"sql(
CREATE TABLE IF NOT EXISTS vessels (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  device_token TEXT UNIQUE NOT NULL
);

CREATE TABLE IF NOT EXISTS embodiments (
  soul_id INTEGER NOT NULL UNIQUE,
  vessel_id INTEGER NOT NULL UNIQUE
);

CREATE TABLE IF NOT EXISTS datings (
  word_id INTEGER PRIMARY KEY,
  created_at_ns INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS supplications (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  suppliant_soul_id INTEGER NOT NULL,
  addressee_soul_id INTEGER NOT NULL,
  asked_at_ns INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS answers (
  supplication_id INTEGER PRIMARY KEY,
  accepted INTEGER NOT NULL,
  answered_at_ns INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS executions (
  deed_word_id INTEGER PRIMARY KEY,
  behest_word_id INTEGER NOT NULL UNIQUE
);
)sql";
		break;
	}

	if (!sql)
		throw std::logic_error("unknown sqlite face");

	check_sqlite(sqlite3_exec(db_, sql, nullptr, nullptr, nullptr), db_, "init_schema");

	if (face_ == SqliteFace::Temporality)
		answer_kept_rejections();
	if (face_ == SqliteFace::Spatiality)
		give_rooms_their_aspect();
}


void SqliteDatabase::give_rooms_their_aspect()
{
	bool has_aspect = false;
	sqlite3_stmt* raw = nullptr;
	check_sqlite(sqlite3_prepare_v2(db_, "PRAGMA table_info(rooms);", -1, &raw, nullptr), db_, "prepare rooms columns");
	while (sqlite3_step(raw) == SQLITE_ROW) {
		const auto* column = reinterpret_cast<const char*>(sqlite3_column_text(raw, 1));
		if (column && std::string_view{column} == "aspect")
			has_aspect = true;
	}
	sqlite3_finalize(raw);
	if (has_aspect)
		return;

	// Rooms kept before they had an aspect were all windows onto words.
	check_sqlite(sqlite3_exec(db_,
							  "BEGIN;"
							  "ALTER TABLE rooms RENAME TO rooms_without_aspect;"
							  "CREATE TABLE rooms (id INTEGER PRIMARY KEY, abode_id INTEGER NOT NULL, "
							  "place_id INTEGER NOT NULL, aspect INTEGER NOT NULL DEFAULT 0, part INTEGER NOT NULL, "
							  "UNIQUE (abode_id, place_id, aspect));"
							  "INSERT INTO rooms (id, abode_id, place_id, aspect, part) "
							  "SELECT id, abode_id, place_id, 0, part FROM rooms_without_aspect;"
							  "DROP TABLE rooms_without_aspect;"
							  "COMMIT;",
							  nullptr, nullptr, nullptr),
				 db_, "give rooms their aspect");
}


void SqliteDatabase::answer_kept_rejections()
{
	sqlite3_stmt* raw = nullptr;
	check_sqlite(sqlite3_prepare_v2(db_, "SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = 'rejections';",
									-1, &raw, nullptr),
				 db_, "prepare rejections lookup");
	const bool kept = sqlite3_step(raw) == SQLITE_ROW;
	sqlite3_finalize(raw);
	if (!kept)
		return;

	check_sqlite(sqlite3_exec(db_,
							  "BEGIN;"
							  "INSERT OR IGNORE INTO answers (supplication_id, accepted, answered_at_ns) "
							  "SELECT supplication_id, 0, rejected_at_ns FROM rejections;"
							  "DROP TABLE rejections;"
							  "COMMIT;",
							  nullptr, nullptr, nullptr),
				 db_, "answer kept rejections");
}


} // namespace will
