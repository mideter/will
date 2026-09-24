#pragma once

#include "acts/creation.h"
#include "sqlite_database.h"
#include "sqlite_eternity.h"
#include "sqlite_spatiality.h"
#include "sqlite_temporality.h"

#include <string>


namespace will {


/** Owns three SQLite faces and Creation of the domain World. */
class SqlitePersistenceBundle {
public:
	/// Opens prefix.eternity.db, prefix.space.db, prefix.time.db.
	explicit SqlitePersistenceBundle(std::string prefix);

	domain::World& world();

	domain::Eternity& eternity() { return eternity_; }
	domain::Spatiality& spatiality() { return spatiality_; }
	domain::Temporality& temporality() { return temporality_; }

private:
	SqliteDatabase eternity_db_;
	SqliteDatabase space_db_;
	SqliteDatabase time_db_;
	SqliteEternity eternity_;
	SqliteSpatiality spatiality_;
	SqliteTemporality temporality_;
	domain::Creation creation_;
};


} // namespace will
