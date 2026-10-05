#include "dweller.h"


namespace will::domain::matter {


Dweller::Dweller(const id::Place abode, const id::Soul soul, const Kind kind)
	: abode_(abode)
	, soul_(soul)
	, kind_(kind)
{}


} // namespace will::domain::matter
