#include "testator.h"

#include "entities/soul.h"
#include "entities/vessel.h"

#include <utility>


namespace will::domain {


Testator::Testator(Man&& man)
	: Executor(std::move(man))
{
	Spirit::present(static_cast<const Soul&>(*this));
	Dust::present(static_cast<const Vessel&>(*this));
}


} // namespace will::domain
