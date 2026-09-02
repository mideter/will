#include "sqlite_persistence_bundle.h"


namespace will {


SqlitePersistenceBundle::SqlitePersistenceBundle(std::string db_path)
    : database_(std::move(db_path))
    , letters_(database_)
    , heaven_(database_)
{}


domain::MessengerPersistence SqlitePersistenceBundle::ports()
{
    return {letters(), heaven()};
}


domain::LetterRepository& SqlitePersistenceBundle::letters()
{
    return letters_;
}


domain::Heaven& SqlitePersistenceBundle::heaven()
{
    return heaven_;
}


} // namespace will
