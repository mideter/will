#pragma once

#include "entities/world.h"
#include "sqlite_database.h"
#include "sqlite_temporality.h"

#include <string>


namespace will {


/** Owns SQLite database and domain World (one Temporality — mutable Eternity). */
class SqlitePersistenceBundle {
public:
	explicit SqlitePersistenceBundle(std::string db_path);

	domain::World& world();

private:
	SqliteDatabase database_;
	SqliteTemporality temporality_;
	domain::World world_;
};


} // namespace will
