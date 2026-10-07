#include "novice.h"

#include "words/training.h"

#include "places/tie.h"
#include "relations/contemplation.h"
#include "men/soul.h"
#include "men/testator.h"
#include "dimensions/spatiality.h"
#include "matter/word.h"
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
	if (!abode().dweller(addressee))
		throw std::logic_error("one supplicates only a dweller of one's abode");

	addressee.hear(Birth<Novice>{*this});
}


std::shared_ptr<const Deed> Novice::execute(const Behest& behest, std::optional<Saying> report,
											 std::optional<std::vector<Exercise>> performed) const
{
	const Tie& tie = behest.tie();
	if (tie.novice().Soul::id() != Soul::id())
		throw std::logic_error("not the novice of this behest");
	if (!contemplates(tie))
		throw std::logic_error("one fulfils a behest only in the tie one contemplates");
	if (!temporality().executions({behest.id()}).empty())
		throw std::logic_error("behest is already executed");

	// A training is fulfilled with what was done, as willed unless told otherwise.
	const auto* training = dynamic_cast<const Training*>(&behest);
	if (!training && performed)
		throw std::logic_error("a plain behest is fulfilled without exercises");
	if (training && !performed)
		performed = training->exercises();
	if (performed && performed->empty())
		throw std::invalid_argument("a training is fulfilled with the exercises done");

	matter::Word uttered = utter(report ? *report : Saying{"совершено"});
	std::optional<matter::Training> done;
	if (performed)
		done = exercise(uttered.id(), std::move(*performed));
	matter::Placement placed = spatiality().place(uttered.id(), tie.id());
	matter::Dating dated = temporality().date(uttered.id());
	matter::Execution executed = temporality().execute(uttered.id(), behest.id());

	return tie.inscribe(
		Birth<Novice>{*this},
		matter::Deed{std::move(uttered), std::move(placed), std::move(dated), std::move(executed), std::move(done)});
}


std::vector<std::reference_wrapper<const Obedience>> Novice::obediences() const
{
	std::vector<std::reference_wrapper<const Obedience>> out;
	out.reserve(obediences_.size());

	for (const auto& place : obediences_)
		out.emplace_back(*place);

	return out;
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

	return follow(std::make_unique<Tie>(Birth<Novice>{*this}, std::move(kept)));
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
