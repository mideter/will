#include "sqlite_persistence_bundle.h"


namespace will {


SqlitePersistenceBundle::SqlitePersistenceBundle(std::string db_path)
    : database_(std::move(db_path))
    , letters_(database_)
    , gods_(database_)
{}


domain::MessengerPersistence SqlitePersistenceBundle::ports()
{
    return {letters(), gods()};
}


domain::LetterRepository& SqlitePersistenceBundle::letters()
{
    return letters_;
}


domain::GodRepository& SqlitePersistenceBundle::gods()
{
    return gods_;
}


} // namespace will
