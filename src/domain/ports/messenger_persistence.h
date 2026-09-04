#pragma once

#include "entities/world.h"
#include "ports/letter_repository.h"


namespace will::domain {


/** Domain persistence ports required by the messenger server runtime. */
struct MessengerPersistence {
	LetterRepository& letters;
	World& world;
};


} // namespace will::domain
