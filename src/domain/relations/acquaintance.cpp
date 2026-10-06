#include "acquaintance.h"


namespace will::domain {


Acquaintance::Acquaintance(Birth<Abode>, const Man& man)
	: man_(man)
{}


bool Acquaintance::enters(matter::Room::Part) const
{
	return false;
}


} // namespace will::domain
