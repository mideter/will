#include "sqlite_persistence_bundle.h"


namespace will {


SqlitePersistenceBundle::SqlitePersistenceBundle(std::string db_path)
	: database_(std::move(db_path))
	, temporality_(database_)
	, creation_(temporality_)
{}


domain::World& SqlitePersistenceBundle::world()
{
	return creation_.world();
}


} // namespace will
