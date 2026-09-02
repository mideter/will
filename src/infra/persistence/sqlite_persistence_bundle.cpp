#include "sqlite_persistence_bundle.h"


namespace will {


SqlitePersistenceBundle::SqlitePersistenceBundle(std::string db_path)
    : database_(std::move(db_path))
    , letters_(database_)
    , users_(database_)
{}


domain::MessengerPersistence SqlitePersistenceBundle::ports()
{
    return {letters(), users()};
}


domain::LetterRepository& SqlitePersistenceBundle::letters()
{
    return letters_;
}


domain::UserRepository& SqlitePersistenceBundle::users()
{
    return users_;
}


} // namespace will
