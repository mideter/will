#include "obedience.h"


namespace will::domain {


Obedience::Obedience(const id::Obedience id)
	: Place(id::Place{id.value()})
{}


} // namespace will::domain
