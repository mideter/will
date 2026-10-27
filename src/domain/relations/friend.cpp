#include "friend.h"


namespace will::domain {


Friend::Friend(const Birth<OuterAbode> birth, const Man& man)
	: Neighbour(birth, man)
{}


bool Friend::sees(const Abode&) const
{
	return true;
}


} // namespace will::domain
