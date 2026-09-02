#pragma once

#include "entities/earth.h"
#include "entities/heaven.h"
#include "ports/letter_repository.h"
#include "ports/eternity.h"


namespace will::domain {


/** Domain persistence ports required by the messenger server runtime. */
struct MessengerPersistence {
    LetterRepository& letters;
    Heaven& heaven;
    Earth& earth;
    Eternity& eternity;
};


} // namespace will::domain
