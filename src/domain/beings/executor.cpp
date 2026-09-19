#include "executor.h"

#include "beings/soul.h"
#include "beings/testator.h"
#include "ports/temporality.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Executor::Executor(Embodiment embodiment)
	: Witness(std::move(embodiment))
{}


void Executor::supplicate(const Testator& testator) const
{
	if (testator.Soul::id() == Soul::id())
		throw std::invalid_argument("cannot supplicate oneself");

	Supplication ask =
		temporality().supplicate(static_cast<const Soul&>(*this),
					 static_cast<const Soul&>(testator));

	testator.receive(std::move(ask));
}


Deed Executor::execute(const Deed& deed) const
{
	if (deed.executor().Soul::id() != Soul::id())
		throw std::logic_error("not the executor of this deed");

	if (!deed.open())
		throw std::logic_error("deed is not open");

	(void)obedience(deed.obedience_id());

	return temporality().execute(deed);
}


const Obedience& Executor::obedience(const id::Obedience id) const
{
	for (const auto& place : obediences_) {
		if (place->obedience_id() == id)
			return *place;
	}
	throw std::invalid_argument("unknown obedience");
}


const Obedience& Executor::keep(Obedience place) const
{
	const id::Obedience id = place.obedience_id();
	for (const auto& existing : obediences_) {
		if (existing->obedience_id() == id)
			return *existing;
	}
	obediences_.push_back(std::make_unique<Obedience>(std::move(place)));
	return *obediences_.back();
}


} // namespace will::domain
