#pragma once

#include "ports/message_repository.h"
#include "ports/user_repository.h"


namespace will::domain {


/** Domain persistence ports required by the messenger server runtime. */
struct MessengerPersistence {
    MessageRepository& messages;
    UserRepository& users;
};


} // namespace will::domain
