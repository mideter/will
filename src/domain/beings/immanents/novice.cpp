#include "novice.h"

#include "beings/immanents/tie.h"
#include "beings/immanents/soul.h"
#include "beings/immanents/testator.h"
#include "ports/spatiality.h"
#include "ports/temporality.h"

#include <memory>
#include <optional>
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

	const Supplication ask{temporality().ask(Soul::id(), addressee.Soul::id())};
	ask.sign(*this);
}


Deed Novice::execute(const Deed& deed) const
{
	if (deed.tie().novice().Soul::id() != Soul::id())
		throw std::logic_error("not the novice of this deed");
	if (!deed.open())
		throw std::logic_error("deed is not open");

	temporality().execute(deed.id());

	return this->deed(deed.id());
}


Deed Novice::deed(const id::Word id) const
{
	const std::optional<matter::Placement> placed = spatiality().placement(id);
	std::vector<matter::Utterance> uttered = utterances({id});
	std::vector<matter::Dating> dated = temporality().datings({id});
	if (!placed || uttered.empty() || dated.empty())
		throw std::invalid_argument("unknown deed");

	std::vector<matter::Execution> executed = temporality().executions({id});
	std::optional<matter::Execution> execution;
	if (!executed.empty())
		execution = std::move(executed.front());

	Deed found{matter::Deed{std::move(uttered.front()), *placed, std::move(dated.front()),
							std::move(execution)}};
	if (found.tie().novice().Soul::id() != Soul::id() && found.tie().testator().Soul::id() != Soul::id())
		throw std::invalid_argument("unknown deed");

	return found;
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

	return follow(std::make_unique<Tie>(std::move(kept)));
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
