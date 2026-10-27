#include "neighbour.h"


namespace will::domain {


Neighbour::Neighbour(const Birth<OuterAbode> birth, const Man& man)
	: Acquaintance(birth, man)
{}


bool Neighbour::sees(const OuterAbode&) const
{
	return true;
}


} // namespace will::domain
