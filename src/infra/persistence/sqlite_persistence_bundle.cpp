#include "sqlite_persistence_bundle.h"


namespace will {


SqlitePersistenceBundle::SqlitePersistenceBundle(std::string db_path)
    : database_(std::move(db_path))
    , store_(database_)
    , letters_(database_)
    , heaven_(store_)
    , earth_(store_)
{}


domain::MessengerPersistence SqlitePersistenceBundle::ports()
{
    return {letters(), heaven(), earth(), eternity()};
}


domain::LetterRepository& SqlitePersistenceBundle::letters()
{
    return letters_;
}


domain::Heaven& SqlitePersistenceBundle::heaven()
{
    return heaven_;
}


domain::Earth& SqlitePersistenceBundle::earth()
{
    return earth_;
}


domain::Eternity& SqlitePersistenceBundle::eternity()
{
    return store_;
}


} // namespace will
