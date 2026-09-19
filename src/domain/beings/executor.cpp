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

	const Obedience& obedience = this->obedience(deed.obedience_id());
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

	const id::Obedience id = obedience.obedience_id();
	temporality().secede(id);

	static_cast<const Executor&>(obedience.executor()).end_obedience(id);
	static_cast<const Testator&>(obedience.testator()).end_shepherding(id);
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


void Executor::end_obedience(const id::Obedience id) const
{
	for (auto& place : obediences_) {
		if (place->obedience_id() == id) {
			place->end();
			return;
		}
	}
	throw std::invalid_argument("unknown obedience");
}


} // namespace will::domain
