#include "dust.h"

#include "entities/vessel.h"


namespace will::domain {


void Dust::present() const
{
	Earth::the().present(static_cast<const Vessel&>(*this));
}


} // namespace will::domain
