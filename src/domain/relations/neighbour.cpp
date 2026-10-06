#include "neighbour.h"


namespace will::domain {


Neighbour::Neighbour(const Birth<Abode> birth, const Man& man)
	: Acquaintance(birth, man)
{}


bool Neighbour::enters(const matter::Room::Part part) const
{
	return part == matter::Room::Part::Outer;
}


} // namespace will::domain
