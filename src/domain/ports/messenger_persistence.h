#pragma once

#include "ports/auth_session_store.h"
#include "ports/message_repository.h"
#include "ports/user_repository.h"


namespace will::domain {


/** Domain persistence ports required by the messenger server runtime. */
struct MessengerPersistence {
    MessageRepository& messages;
    UserRepository& users;
    AuthSessionStore& sessions;
};


} // namespace will::domain
