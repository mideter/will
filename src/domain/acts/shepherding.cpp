#include "shepherding.h"


namespace will::domain {


Shepherding::Shepherding(const id::Tie id)
	: Place(id::Place{id.value()})
{}


} // namespace will::domain
