#include "sqlite_persistence_bundle.h"


namespace will {


SqlitePersistenceBundle::SqlitePersistenceBundle(std::string db_path)
	: database_(std::move(db_path))
	, eternity_(database_)
	, temporality_(database_, eternity_.time())
	, world_(eternity_, temporality_)
{}


domain::World& SqlitePersistenceBundle::world()
{
	return world_;
}


} // namespace will
