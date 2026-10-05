#include "neighbour.h"


namespace will::domain {


Neighbour::Neighbour(const Birth<Abode> birth, const Man& man)
	: Acquaintance(birth, man)
{}


bool Neighbour::beholds(const Word&) const
{
	return true;
}


} // namespace will::domain
