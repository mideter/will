#include "novice.h"

#include "immanents/tie.h"
#include "immanents/soul.h"
#include "immanents/testator.h"
#include "dimensions/temporality.h"

#include <memory>
#include <stdexcept>
#include <utility>


namespace will::domain {


Novice::Novice(matter::Man kept)
	: Witness(std::move(kept))
{}


void Novice::supplicate(const Testator& addressee) const
{
	if (addressee.Soul::id() == Soul::id())
		throw std::invalid_argument("cannot supplicate oneself");
	if (follows(addressee))
		throw std::logic_error("obedience already exists for this pair");

	const Supplication ask{Birth<Novice>{}, temporality().ask(Soul::id(), addressee.Soul::id())};
	ask.sign(*this);
}


void Novice::execute(const Deed& deed) const
{
	if (deed.tie().novice().Soul::id() != Soul::id())
		throw std::logic_error("not the novice of this deed");
	if (!deed.open())
		throw std::logic_error("deed is not open");

	temporality().execute(deed.id());
}


std::vector<std::reference_wrapper<const Obedience>> Novice::obediences() const
{
	std::vector<std::reference_wrapper<const Obedience>> out;
	out.reserve(obediences_.size());

	for (const auto& place : obediences_)
		out.emplace_back(*place);

	return out;
}


bool Novice::follows(const Testator& testator) const noexcept
{
	for (const auto& place : obediences_) {
		if (place->testator().Soul::id() == testator.Soul::id())
			return true;
	}

	return false;
}


const Obedience& Novice::obedience(const Testator& testator) const
{
	for (const auto& place : obediences_) {
		if (place->testator().Soul::id() == testator.Soul::id())
			return *place;
	}

	throw std::invalid_argument("unknown obedience");
}


const Obedience& Novice::follow(matter::Tie kept) const
{
	if (kept.novice() != Soul::id())
		throw std::logic_error("tie is not for this novice");

	return follow(std::make_unique<Tie>(Birth<Novice>{}, std::move(kept)));
}


const Obedience& Novice::follow(std::unique_ptr<Obedience> place) const
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
