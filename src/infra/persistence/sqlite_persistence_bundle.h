#pragma once

#include "horizons/creation.h"
#include "horizons/life.h"
#include "sqlite_eternity.h"

#include <string>


namespace will {


/** Owns SQLite Eternity and Life, which holds the Creation of the domain World. */
class SqlitePersistenceBundle {
public:
	/// Opens will.eternity.db in this directory; Creation opens will.space.db and
	/// will.time.db there.
	explicit SqlitePersistenceBundle(std::string directory);

	domain::World& world();

	domain::Eternity& eternity() { return eternity_; }
	domain::Spatiality& spatiality() { return life_.creation().spatiality(); }
	domain::Temporality& temporality() { return life_.creation().temporality(); }

private:
	SqliteEternity eternity_;
	domain::Life life_;
};


} // namespace will
