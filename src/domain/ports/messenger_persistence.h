#pragma once

#include "entities/heaven.h"
#include "ports/letter_repository.h"


namespace will::domain {


/** Domain persistence ports required by the messenger server runtime. */
struct MessengerPersistence {
    LetterRepository& letters;
    Heaven& heaven;
};


} // namespace will::domain
