#include "sqlite_persistence_bundle.h"

#include <string>
#include <utility>


namespace will {


SqlitePersistenceBundle::SqlitePersistenceBundle(std::string directory)
	: eternity_(std::move(directory))
{
	life_.create();
}


domain::World& SqlitePersistenceBundle::world()
{
	return life_.creation().world();
}


} // namespace will
