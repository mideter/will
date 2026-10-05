#include "acquaintance.h"


namespace will::domain {


Acquaintance::Acquaintance(Birth<Abode>, const Man& man)
	: man_(man)
{}


bool Acquaintance::beholds(const Word&) const
{
	return false;
}


} // namespace will::domain
