#pragma once

#include "acts/creation.h"
#include "sqlite_database.h"
#include "sqlite_temporality.h"

#include <string>


namespace will {


/** Owns SQLite database and Creation of the domain World. */
class SqlitePersistenceBundle {
public:
	explicit SqlitePersistenceBundle(std::string db_path);

	domain::World& world();

private:
	SqliteDatabase database_;
	SqliteTemporality temporality_;
	domain::Creation creation_;
};


} // namespace will
