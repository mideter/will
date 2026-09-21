#include "obedience.h"


namespace will::domain {


Obedience::Obedience(const id::Tie id)
	: Place(id::Place{id.value()})
{}


} // namespace will::domain
