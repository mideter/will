#pragma once

#include "entities/earth.h"
#include "entities/heaven.h"
#include "ports/letter_repository.h"


namespace will::domain {


/** Domain persistence ports required by the messenger server runtime. */
struct MessengerPersistence {
    LetterRepository& letters;
    Heaven& heaven;
    Earth& earth;
};


} // namespace will::domain
