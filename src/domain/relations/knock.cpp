#include "knock.h"

#include "places/gates.h"


namespace will::domain {


Knock::Knock(const Birth<Heaven> birth, const Man& who, const Gates& gates)
	: Contemplation(birth, who, gates)
{}


} // namespace will::domain
