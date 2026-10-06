#include "unborn.h"

#include <utility>


namespace will::domain {


Unborn::Unborn(Birth<World>, matter::Vessel kept)
	: Vessel(std::move(kept))
{
	Immanent<Earth>::present<Vessel>();
}


} // namespace will::domain
