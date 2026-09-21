#include "shepherding.h"


namespace will::domain {


Shepherding::Shepherding(const id::Obedience id)
	: Place(id::Place{id.value()})
{}


} // namespace will::domain
