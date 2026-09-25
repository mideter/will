#include "sqlite_database.h"

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

CREATE TABLE IF NOT EXISTS utterances (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  author_soul_id INTEGER NOT NULL,
  body TEXT NOT NULL
);
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

CREATE TABLE IF NOT EXISTS placements (
  letter_id INTEGER PRIMARY KEY,
  place_id INTEGER NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_placements_place ON placements(place_id);

CREATE TABLE IF NOT EXISTS place_hwm (
  id INTEGER PRIMARY KEY CHECK (id = 1),
  value INTEGER NOT NULL
);
INSERT OR IGNORE INTO place_hwm (id, value) VALUES (1, 0);
)sql";
		break;
	case SqliteFace::Temporality:
		sql = R"sql(
CREATE TABLE IF NOT EXISTS vessels (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  device_token TEXT UNIQUE NOT NULL
);

CREATE TABLE IF NOT EXISTS men (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  soul_id INTEGER NOT NULL UNIQUE,
  vessel_id INTEGER NOT NULL UNIQUE,
  soul_name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS datings (
  letter_id INTEGER PRIMARY KEY,
  created_at_ns INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS supplications (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  suppliant_soul_id INTEGER NOT NULL,
  testator_soul_id INTEGER NOT NULL,
  status TEXT NOT NULL,
  created_at_ns INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS obediences (
  id INTEGER PRIMARY KEY,
  testator_soul_id INTEGER NOT NULL,
  novice_soul_id INTEGER NOT NULL,
  created_at_ns INTEGER NOT NULL,
  seceded_at_ns INTEGER
);

CREATE TABLE IF NOT EXISTS deeds (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  obedience_id INTEGER NOT NULL,
  testator_soul_id INTEGER NOT NULL,
  novice_soul_id INTEGER NOT NULL,
  body TEXT NOT NULL,
  created_at_ns INTEGER NOT NULL,
  executed_at_ns INTEGER,
  cancelled_at_ns INTEGER
);
)sql";
		break;
	}

	if (!sql)
		throw std::logic_error("unknown sqlite face");

	check_sqlite(sqlite3_exec(db_, sql, nullptr, nullptr, nullptr), db_, "init_schema");
}


} // namespace will
