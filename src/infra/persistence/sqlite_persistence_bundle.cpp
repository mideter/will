#include "sqlite_persistence_bundle.h"

#include <string>
#include <utility>


namespace will {


SqlitePersistenceBundle::SqlitePersistenceBundle(std::string prefix)
	: eternity_(std::move(prefix))
	, creation_(life_.create())
{}


domain::World& SqlitePersistenceBundle::world()
{
	return creation_.world();
}


} // namespace will
