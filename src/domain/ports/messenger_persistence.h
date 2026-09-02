#pragma once

#include "ports/letter_repository.h"
#include "ports/user_repository.h"


namespace will::domain {


/** Domain persistence ports required by the messenger server runtime. */
struct MessengerPersistence {
    LetterRepository& letters;
    UserRepository& users;
};


} // namespace will::domain
