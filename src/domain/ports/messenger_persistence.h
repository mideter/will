#pragma once

#include "ports/god_repository.h"
#include "ports/letter_repository.h"


namespace will::domain {


/** Domain persistence ports required by the messenger server runtime. */
struct MessengerPersistence {
    LetterRepository& letters;
    GodRepository& gods;
};


} // namespace will::domain
