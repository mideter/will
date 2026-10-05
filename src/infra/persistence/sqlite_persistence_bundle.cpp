#include "sqlite_persistence_bundle.h"

#include <string>
#include <utility>


namespace will {


SqlitePersistenceBundle::SqlitePersistenceBundle(std::string prefix)
	: eternity_(std::move(prefix))
{
	life_.create();
}


domain::World& SqlitePersistenceBundle::world()
{
	return life_.creation().world();
}


} // namespace will
