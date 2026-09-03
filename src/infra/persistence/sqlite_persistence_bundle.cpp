#include "sqlite_persistence_bundle.h"


namespace will {


SqlitePersistenceBundle::SqlitePersistenceBundle(std::string db_path)
    : database_(std::move(db_path))
    , store_(database_)
    , letters_(database_)
    , heaven_(store_)
    , earth_(heaven_)
{}


domain::MessengerPersistence SqlitePersistenceBundle::ports()
{
    return {letters(), heaven(), earth()};
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


} // namespace will
