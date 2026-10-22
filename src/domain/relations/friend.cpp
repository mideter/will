#include "friend.h"


namespace will::domain {


Friend::Friend(const Birth<Abode> birth, const Man& man)
	: Neighbour(birth, man)
{}


bool Friend::enters(const Inner&) const
{
	return true;
}


} // namespace will::domain
