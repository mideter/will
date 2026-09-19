#include "testator.h"

#include "beings/soul.h"
#include "ports/temporality.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Testator::Testator(Embodiment embodiment)
	: Executor(std::move(embodiment))
{}


const Obedience& Testator::accept(const Supplication& supplication) const
{
	if (supplication.addressee().id() != Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");
	if (supplication.status() != SupplicationStatus::pending)
		throw std::logic_error("supplication is not pending");

	const Obedience created = temporality().accept(supplication.id());
	const Soul& testator_soul = created.testator();
	const Soul& executor_soul = created.executor();
	const id::Obedience id = created.obedience_id();

	keep(Shepherding{id, testator_soul, executor_soul});
	return static_cast<const Executor&>(executor_soul)
		.keep(Obedience{id, testator_soul, executor_soul});
}


void Testator::refuse(const Supplication& supplication) const
{
	if (supplication.addressee().id() != Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");
	if (supplication.status() != SupplicationStatus::pending)
		throw std::logic_error("supplication is not pending");

	temporality().refuse(supplication.id());
}


Deed Testator::will(const Shepherding& shepherding, const Word& word) const
{
	if (shepherding.testator().id() != Soul::id())
		throw std::logic_error("not the testator of this shepherding");

	const Obedience face{shepherding.obedience_id(), shepherding.testator(), shepherding.executor()};
	return temporality().bequeath(face, static_cast<const Soul&>(*this), word);
}


const Shepherding& Testator::shepherding(const id::Obedience id) const
{
	for (const auto& place : shepherdings_) {
		if (place->obedience_id() == id)
			return *place;
	}
	throw std::invalid_argument("unknown shepherding");
}


const Shepherding& Testator::keep(Shepherding place) const
{
	const id::Obedience id = place.obedience_id();
	for (const auto& existing : shepherdings_) {
		if (existing->obedience_id() == id)
			return *existing;
	}
	shepherdings_.push_back(std::make_unique<Shepherding>(std::move(place)));
	return *shepherdings_.back();
}


} // namespace will::domain
