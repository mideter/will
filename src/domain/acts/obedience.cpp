#include "obedience.h"


namespace will::domain {


Obedience::Obedience(AsTieFace, const id::Obedience id)
	: Place(id::Place{id.value()})
{}


} // namespace will::domain
