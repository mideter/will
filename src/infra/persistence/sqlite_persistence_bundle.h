#pragma once

#include "acts/creation.h"
#include "horizons/life.h"
#include "sqlite_eternity.h"

#include <string>


namespace will {


/** Owns SQLite Eternity, Life, and the Creation of the domain World. */
class SqlitePersistenceBundle {
public:
	/// Opens prefix.eternity.db; Creation opens prefix.space.db, prefix.time.db.
	explicit SqlitePersistenceBundle(std::string prefix);

	domain::World& world();

	domain::Eternity& eternity() { return eternity_; }
	domain::Spatiality& spatiality() { return creation_.spatiality(); }
	domain::Temporality& temporality() { return creation_.temporality(); }

private:
	SqliteEternity eternity_;
	domain::Life life_;
	domain::Creation creation_;
};


} // namespace will
