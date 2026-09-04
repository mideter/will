#include "sqlite_persistence_bundle.h"


namespace will {


SqlitePersistenceBundle::SqlitePersistenceBundle(std::string db_path)
	: database_(std::move(db_path))
	, store_(database_)
	, letters_(database_)
	, world_(store_)
{}


domain::MessengerPersistence SqlitePersistenceBundle::ports()
{
	return {letters(), world()};
}


domain::LetterRepository& SqlitePersistenceBundle::letters()
{
	return letters_;
}


domain::World& SqlitePersistenceBundle::world()
{
	return world_;
}


} // namespace will
