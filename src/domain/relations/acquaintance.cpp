#include "acquaintance.h"


namespace will::domain {


Acquaintance::Acquaintance(Birth<Abode>, const Man& man)
	: man_(man)
{}


bool Acquaintance::enters(const Outer&) const
{
	return false;
}


bool Acquaintance::enters(const Inner&) const
{
	return false;
}


} // namespace will::domain
