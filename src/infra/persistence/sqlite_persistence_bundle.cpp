#include "sqlite_persistence_bundle.h"


namespace will {


SqlitePersistenceBundle::SqlitePersistenceBundle(std::string db_path)
    : database_(std::move(db_path))
    , messages_(database_)
    , users_(database_)
    , sessions_(database_)
    , otp_store_(database_)
{}


domain::MessengerPersistence SqlitePersistenceBundle::ports()
{
    return {messages(), users(), sessions()};
}


domain::MessageRepository& SqlitePersistenceBundle::messages()
{
    return messages_;
}


domain::UserRepository& SqlitePersistenceBundle::users()
{
    return users_;
}


domain::AuthSessionStore& SqlitePersistenceBundle::sessions()
{
    return sessions_;
}


domain::OtpStore& SqlitePersistenceBundle::otp_store()
{
    return otp_store_;
}


} // namespace will
