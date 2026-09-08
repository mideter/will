#include "shelter.h"


namespace will::domain {


Shelter::Shelter(Abode& abode, Man& man) noexcept
	: abode_(abode)
	, man_(man)
{}


} // namespace will::domain
