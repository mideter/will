#include "friend.h"


namespace will::domain {


Friend::Friend(const Birth<Abode> birth, const Man& man)
	: Neighbour(birth, man)
{}


} // namespace will::domain
