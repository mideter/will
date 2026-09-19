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

	return temporality().supplicate(static_cast<const Soul&>(*this), testator);
}


Deed Executor::execute(const Deed& deed) const
{
	if (deed.executor().id() != Soul::id())
		throw std::logic_error("not the executor of this deed");

	if (!deed.open())
		throw std::logic_error("deed is not open");

	const Obedience obedience = temporality().obedience(deed.obedience_id());
	if (!obedience.living())
		throw std::logic_error("obedience is not living");

	return temporality().execute(deed);
}


void Executor::secede(const Obedience& obedience) const
{
	if (obedience.testator().id() != Soul::id() && obedience.executor().id() != Soul::id())
		throw std::logic_error("not a party to this obedience");
	if (!obedience.living())
		throw std::logic_error("obedience is not living");

	temporality().secede(obedience.obedience_id());
}


} // namespace will::domain
