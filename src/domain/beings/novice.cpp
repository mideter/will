#include "novice.h"

#include "acts/tie.h"
#include "beings/soul.h"
#include "beings/testator.h"
#include "ports/temporality.h"

#include <memory>
#include <stdexcept>
#include <utility>


namespace will::domain {


Novice::Novice(Embodiment embodiment)
	: Witness(std::move(embodiment))
{}


void Novice::supplicate(const Testator& addressee) const
{
	if (addressee.Soul::id() == Soul::id())
		throw std::invalid_argument("cannot supplicate oneself");

	Supplication ask = temporality().supplicate(*this, addressee);
	ask.sign(*this);
}


Deed Novice::execute(const Deed& deed) const
{
	if (deed.tie().novice().Soul::id() != Soul::id())
		throw std::logic_error("not the novice of this deed");
	if (!deed.open())
		throw std::logic_error("deed is not open");

	(void)obedience(id::Tie{deed.tie().id()});

	return temporality().execute(deed);
}


const Obedience& Novice::obedience(const id::Tie id) const
{
	for (const auto& place : obediences_) {
		if (place->id().value() == id.value())
			return *place;
	}

	throw std::invalid_argument("unknown obedience");
}


const Obedience& Novice::keep(Tying tying) const
{
	if (tying.novice() != Soul::id())
		throw std::logic_error("tying is not for this novice");

	return keep(std::make_unique<Tie>(std::move(tying)));
}


const Obedience& Novice::keep(std::unique_ptr<Obedience> place) const
{
	if (!place)
		throw std::invalid_argument("obedience required");

	const id::Place id = place->id();

	for (const auto& existing : obediences_) {
		if (existing->id() == id)
			return *existing;
	}

	obediences_.push_back(std::move(place));
	return *obediences_.back();
}


} // namespace will::domain
