#include "novice.h"

#include <algorithm>

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


namespace {


/// What was done of a training, by its efforts: the exercises in their willed order,
/// each with its approaches in theirs; an exercise of which nothing was done is left out.
std::vector<Exercise> done_of(const Training& training)
{
	std::vector<matter::Effort> efforts = training.efforts();
	std::sort(efforts.begin(), efforts.end(), [](const matter::Effort& a, const matter::Effort& b) {
		return a.exercise() != b.exercise() ? a.exercise() < b.exercise() : a.approach() < b.approach();
	});

	std::vector<Exercise> done;
	for (std::uint32_t e = 0; e < training.exercises().size(); ++e) {
		const Exercise& willed = training.exercises()[e];
		std::vector<Approach> approaches;
		for (const matter::Effort& effort : efforts) {
			if (effort.exercise() != e)
				continue;
			const std::uint32_t rest = effort.approach() < willed.approaches().size()
										   ? willed.approaches()[effort.approach()].rest_seconds()
										   : 0;
			approaches.emplace_back(effort.weight(), effort.repetitions(), rest);
		}
		if (!approaches.empty())
			done.emplace_back(willed.name(), std::move(approaches));
	}
	return done;
}


} // namespace


void Novice::begin(const Training& training, const std::uint32_t exercise, const std::uint32_t approach) const
{
	const Tie& tie = training.tie();
	if (tie.novice().Soul::id() != Soul::id())
		throw std::logic_error("not the novice of this behest");
	if (!contemplates(tie))
		throw std::logic_error("one does a training only in the tie one contemplates");
	if (!temporality().executions({training.id()}).empty())
		throw std::logic_error("behest is already executed");
	if (exercise >= training.exercises().size())
		throw std::invalid_argument("no such exercise");

	// The approaches of an exercise go in order: the first not yet done; once the
	// willed are done, one more beyond them at a time. Exercises go in any order.
	const auto willed = static_cast<std::uint32_t>(training.exercises()[exercise].approaches().size());
	std::uint32_t count = willed;
	for (const matter::Effort& effort : training.efforts()) {
		if (effort.exercise() == exercise && effort.approach() >= count)
			count = effort.approach() + 1;
	}
	std::uint32_t next = 0;
	while (next < count && training.done(exercise, next))
		++next;
	if (approach > count)
		throw std::invalid_argument("no such approach");
	if (training.done(exercise, approach))
		throw std::logic_error("the approach is already done");
	if (approach != next)
		throw std::logic_error("the approaches of an exercise are done in order");

	std::lock_guard lock(underway_mutex_);
	if (underway_)
		throw std::logic_error("another approach is underway");
	underway_ = Underway{training.id(), exercise, approach, now()};
}


matter::Effort Novice::finish(const Training& training, const Weight weight, const std::uint32_t repetitions) const
{
	if (!contemplates(training.tie()))
		throw std::logic_error("one does a training only in the tie one contemplates");

	Underway doing{training.id(), 0, 0, Timestamp{0}};
	{
		std::lock_guard lock(underway_mutex_);
		if (!underway_ || underway_->training != training.id())
			throw std::logic_error("no approach of this training is underway");
		doing = *underway_;
	}

	matter::Effort kept
		= temporality().exert(training.id(), doing.exercise, doing.approach, weight, repetitions, doing.begun);
	training.exert(Birth<Novice>{*this}, kept);
	{
		std::lock_guard lock(underway_mutex_);
		underway_.reset();
	}
	return kept;
}


std::optional<Novice::Underway> Novice::underway() const
{
	std::lock_guard lock(underway_mutex_);
	return underway_;
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

	// A training is fulfilled with what was done: its efforts, unless told otherwise.
	const auto* training = dynamic_cast<const Training*>(&behest);
	if (!training && performed)
		throw std::logic_error("a plain behest is fulfilled without exercises");
	if (training) {
		if (const auto doing = underway(); doing && doing->training == training->id())
			throw std::logic_error("finish the approach underway first");
		if (!performed)
			performed = done_of(*training);
	}
	if (performed && performed->empty())
		performed.reset();  // nothing was done: the deed tells no exercises

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
