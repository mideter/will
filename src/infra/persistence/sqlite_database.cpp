#include "sqlite_database.h"

#include "sqlite_util.h"

#include <cstring>
#include <format>
#include <sqlite3.h>


namespace will {


SqliteDatabase::SqliteDatabase(std::string db_path)
	: db_path_(std::move(db_path))
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


namespace {


bool table_has_column(sqlite3* db, const char* table, const char* column)
{
	const std::string pragma = std::format("PRAGMA table_info({});", table);
	sqlite3_stmt* stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db, pragma.c_str(), -1, &stmt, nullptr), db, "prepare table_info");

	bool found = false;
	int rc = sqlite3_step(stmt);
	while (rc == SQLITE_ROW) {
		const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
		if (name && std::strcmp(name, column) == 0)
			found = true;
		rc = sqlite3_step(stmt);
	}

	check_sqlite(rc, db, "table_info step");
	sqlite3_finalize(stmt);
	return found;
}


bool table_exists(sqlite3* db, const char* table)
{
	sqlite3_stmt* stmt = nullptr;
	check_sqlite(sqlite3_prepare_v2(db,
									"SELECT 1 FROM sqlite_master WHERE type='table' AND name=? LIMIT 1;",
									-1, &stmt, nullptr),
				 db, "prepare table_exists");
	check_sqlite(sqlite3_bind_text(stmt, 1, table, -1, SQLITE_STATIC), db, "bind table_exists");
	const int rc = sqlite3_step(stmt);
	const bool exists = rc == SQLITE_ROW;
	sqlite3_finalize(stmt);
	check_sqlite(rc == SQLITE_ROW || rc == SQLITE_DONE ? SQLITE_OK : rc, db, "table_exists step");
	return exists;
}


bool needs_schema_reset(sqlite3* db)
{
	if (table_exists(db, "messages"))
		return true;
	if (table_exists(db, "users"))
		return true;
	if (table_has_column(db, "letters", "author_user_id"))
		return true;
	if (table_has_column(db, "messages", "chat_id"))
		return true;
	if (table_has_column(db, "messages", "sender_ip"))
		return true;
	if (table_has_column(db, "messages", "created_at_ms"))
		return true;
	if (table_has_column(db, "users", "password_hash"))
		return true;
	if (table_has_column(db, "users", "phone"))
		return true;
	if (table_has_column(db, "souls", "password_hash") || table_has_column(db, "gods", "password_hash"))
		return true;
	if (table_has_column(db, "souls", "phone") || table_has_column(db, "gods", "phone"))
		return true;

	const bool has_souls = table_exists(db, "souls");
	const bool has_gods = table_exists(db, "gods");

	return !has_souls && !has_gods && table_has_column(db, "letters", "body");
}


void drop_legacy_tables(sqlite3* db)
{
	static constexpr const char* DropLegacyTablesSql = R"sql(
DROP TABLE IF EXISTS letters;
DROP TABLE IF EXISTS men;
DROP TABLE IF EXISTS vessels;
DROP TABLE IF EXISTS messages;
DROP TABLE IF EXISTS auth_sessions;
DROP TABLE IF EXISTS otp_challenges;
DROP TABLE IF EXISTS souls;
DROP TABLE IF EXISTS gods;
DROP TABLE IF EXISTS users;
)sql";
	check_sqlite(sqlite3_exec(db, DropLegacyTablesSql, nullptr, nullptr, nullptr), db,
				 "drop_legacy_tables");
}


void migrate_souls_device_token_to_vessels(sqlite3* db)
{
	if (!table_has_column(db, "souls", "device_token"))
		return;

	check_sqlite(
		sqlite3_exec(db, "INSERT OR IGNORE INTO vessels (device_token) SELECT device_token FROM souls;",
					 nullptr, nullptr, nullptr),
		db, "migrate device_token to vessels");

	check_sqlite(
		sqlite3_exec(db,
					 "INSERT OR IGNORE INTO men (soul_id, vessel_id) "
					 "SELECT s.id, v.id FROM souls s "
					 "JOIN vessels v ON v.device_token = s.device_token;",
					 nullptr, nullptr, nullptr),
		db, "migrate souls/vessels to men");

	check_sqlite(sqlite3_exec(db, "BEGIN IMMEDIATE;", nullptr, nullptr, nullptr), db,
				 "begin migrate souls schema");
	check_sqlite(sqlite3_exec(db,
							  "CREATE TABLE souls_new ("
							  "  id INTEGER PRIMARY KEY,"
							  "  name TEXT NOT NULL DEFAULT ''"
							  ");",
							  nullptr, nullptr, nullptr),
				 db, "create souls_new");
	check_sqlite(sqlite3_exec(db, "INSERT INTO souls_new (id, name) SELECT id, name FROM souls;", nullptr, nullptr,
							  nullptr),
				 db, "copy souls to souls_new");
	check_sqlite(sqlite3_exec(db, "DROP TABLE souls;", nullptr, nullptr, nullptr), db, "drop old souls");
	check_sqlite(sqlite3_exec(db, "ALTER TABLE souls_new RENAME TO souls;", nullptr, nullptr, nullptr), db,
				 "rename souls_new");
	check_sqlite(sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr), db, "commit migrate souls schema");
}


void migrate_gods_to_souls(sqlite3* db)
{
	if (table_exists(db, "gods") && !table_exists(db, "souls"))
		check_sqlite(sqlite3_exec(db, "ALTER TABLE gods RENAME TO souls;", nullptr, nullptr, nullptr), db,
					 "rename gods to souls");

	if (table_has_column(db, "vessels", "god_id") && !table_has_column(db, "vessels", "soul_id"))
		check_sqlite(sqlite3_exec(db, "ALTER TABLE vessels RENAME COLUMN god_id TO soul_id;", nullptr, nullptr,
								  nullptr),
					 db, "rename vessels.god_id");

	if (table_has_column(db, "letters", "author_god_id") && !table_has_column(db, "letters", "author_soul_id"))
		check_sqlite(sqlite3_exec(db, "ALTER TABLE letters RENAME COLUMN author_god_id TO author_soul_id;",
								  nullptr, nullptr, nullptr),
					 db, "rename letters.author_god_id");
}


void migrate_vessels_soul_id_to_men(sqlite3* db)
{
	if (!table_has_column(db, "vessels", "soul_id"))
		return;

	check_sqlite(
		sqlite3_exec(db,
					 "INSERT OR IGNORE INTO men (soul_id, vessel_id) "
					 "SELECT soul_id, id FROM vessels;",
					 nullptr, nullptr, nullptr),
		db, "migrate vessels.soul_id to men");

	check_sqlite(sqlite3_exec(db, "BEGIN IMMEDIATE;", nullptr, nullptr, nullptr), db,
				 "begin migrate vessels schema");
	check_sqlite(sqlite3_exec(db,
							  "CREATE TABLE vessels_new ("
							  "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
							  "  device_token TEXT UNIQUE NOT NULL"
							  ");",
							  nullptr, nullptr, nullptr),
				 db, "create vessels_new");
	check_sqlite(sqlite3_exec(db,
							  "INSERT INTO vessels_new (id, device_token) SELECT id, device_token FROM vessels;",
							  nullptr, nullptr, nullptr),
				 db, "copy vessels to vessels_new");
	check_sqlite(sqlite3_exec(db, "DROP TABLE vessels;", nullptr, nullptr, nullptr), db, "drop old vessels");
	check_sqlite(sqlite3_exec(db, "ALTER TABLE vessels_new RENAME TO vessels;", nullptr, nullptr, nullptr), db,
				 "rename vessels_new");
	check_sqlite(sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr), db, "commit migrate vessels schema");
}


} // namespace


void SqliteDatabase::init_schema()
{
	if (needs_schema_reset(db_))
		drop_legacy_tables(db_);

	// Rename legacy gods* identifiers before CREATE IF NOT EXISTS, so we do not
	// create an empty souls table alongside an existing gods table.
	migrate_gods_to_souls(db_);

	static constexpr const char* InitSchemaSql = R"sql(
CREATE TABLE IF NOT EXISTS souls (
  id INTEGER PRIMARY KEY,
  name TEXT NOT NULL DEFAULT ''
);

CREATE TABLE IF NOT EXISTS vessels (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  device_token TEXT UNIQUE NOT NULL
);

CREATE TABLE IF NOT EXISTS men (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  soul_id INTEGER NOT NULL REFERENCES souls(id),
  vessel_id INTEGER NOT NULL UNIQUE REFERENCES vessels(id)
);

CREATE TABLE IF NOT EXISTS letters (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  abode_id INTEGER NOT NULL,
  author_soul_id INTEGER NOT NULL REFERENCES souls(id),
  body TEXT NOT NULL,
  created_at_ns INTEGER NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_letters_created_at ON letters(created_at_ns);
)sql";

	check_sqlite(sqlite3_exec(db_, InitSchemaSql, nullptr, nullptr, nullptr), db_, "init_schema");

	migrate_souls_device_token_to_vessels(db_);
	// Column renames for DBs that already had vessels/letters before gods→souls.
	migrate_gods_to_souls(db_);
	migrate_vessels_soul_id_to_men(db_);

	check_sqlite(sqlite3_exec(db_, "UPDATE letters SET abode_id = 1 WHERE abode_id = 0;", nullptr, nullptr,
							  nullptr),
				 db_, "migrate global abode_id");

	if (table_has_column(db_, "souls", "id") && !table_has_column(db_, "souls", "name")) {
		check_sqlite(sqlite3_exec(db_, "ALTER TABLE souls ADD COLUMN name TEXT NOT NULL DEFAULT '';", nullptr,
								  nullptr, nullptr),
					 db_, "add souls.name");
	}
}


} // namespace will
