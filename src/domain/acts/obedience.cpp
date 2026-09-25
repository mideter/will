#include "obedience.h"


namespace will::domain {


Obedience::Obedience(const id::Tie id, const Testator& testator)
	: Place(id::Place{id.value()})
	, testator_(testator)
{}


} // namespace will::domain
