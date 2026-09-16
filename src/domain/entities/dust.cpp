#include "dust.h"

#include "entities/vessel.h"


namespace will::domain {


void Dust::index(const Vessel& vessel) const
{
	Earth::the().index(vessel);
}


} // namespace will::domain
