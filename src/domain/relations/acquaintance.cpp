#include "acquaintance.h"


namespace will::domain {


Acquaintance::Acquaintance(Birth<OuterAbode>, const Man& man)
	: man_(man)
{}


bool Acquaintance::sees(const OuterAbode&) const
{
	return false;
}


bool Acquaintance::sees(const Abode&) const
{
	return false;
}


} // namespace will::domain
