module;

#include <string>

module will.persistence.sqlite_persistence_bundle;


namespace will {


SqlitePersistenceBundle::SqlitePersistenceBundle(std::string db_path)
    : database_(std::move(db_path))
    , messages_(database_)
    , users_(database_)
{}


domain::MessengerPersistence SqlitePersistenceBundle::ports()
{
    return {messages(), users()};
}


domain::MessageRepository& SqlitePersistenceBundle::messages()
{
    return messages_;
}


domain::UserRepository& SqlitePersistenceBundle::users()
{
    return users_;
}


} // namespace will
