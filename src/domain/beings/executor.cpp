#include "executor.h"

#include "beings/soul.h"
#include "ports/temporality.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Executor::Executor(Embodiment embodiment)
	: Witness(std::move(embodiment))
{}


Supplication Executor::supplicate(const Soul& testator) const
{
	if (testator.id() == Soul::id())
		throw std::invalid_argument("cannot supplicate oneself");

	if (!heaven().knows(testator.id()))
		throw std::invalid_argument("unknown testator soul");

	return temporality().supplicate(static_cast<const Soul&>(*this), testator);
}


void Executor::secede(const Obedience& obedience) const
{
	if (obedience.testator() != Soul::id() && obedience.executor() != Soul::id())
		throw std::logic_error("not a party to this obedience");
	if (!obedience.living())
		throw std::logic_error("obedience is not living");

	temporality().secede(obedience.obedience_id());
}


} // namespace will::domain
