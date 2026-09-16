#include "dust.h"

#include "entities/vessel.h"


namespace will::domain {


void Dust::present(const Vessel& vessel) const
{
	Earth::the().present(vessel);
}


} // namespace will::domain
