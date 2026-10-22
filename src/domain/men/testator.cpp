#include "testator.h"
#include "men/man.h"
#include "refusals/tie.h"
#include "refusals/testator.h"

#include "words/training.h"
#include "words/letter.h"
#include "places/abode.h"
#include "places/room.h"
#include "places/tie.h"

#include "matter/dating.h"
#include "matter/placement.h"
#include "matter/word.h"
#include "relations/contemplation.h"
#include "men/soul.h"
#include "dimensions/spatiality.h"
#include "dimensions/temporality.h"

#include <algorithm>
#include <stdexcept>
#include <utility>


namespace will::domain {


const Shepherding& Testator::accept(const Birth<UpperRoom> upper_room, const Supplication& ask) const
{
	if (&upper_room.parent().abode() != &abode())
		throw std::logic_error("one answers in one's own upper room");

	return ask.accept(*this);
}


void Testator::reject(const Birth<UpperRoom> upper_room, const Supplication& ask) const
{
	if (&upper_room.parent().abode() != &abode())
		throw std::logic_error("one answers in one's own upper room");

	ask.reject(*this);
}


std::shared_ptr<const Behest> Testator::will(Birth<Cell>, const Shepherding& shepherding, const Saying& saying) const
{
	matter::Word uttered = utter(saying);
	matter::Placement placed = spatiality().place(uttered.id(), shepherding.id());
	matter::Dating dated = temporality().date(uttered.id());

	return shepherding.inscribe(
		Birth<Testator>{*this}, matter::Behest{std::move(uttered), std::move(placed), std::move(dated)});
}


std::shared_ptr<const Training> Testator::train(Birth<Cell>, const Shepherding& shepherding, const Saying& title,
												std::vector<Exercise> exercises) const
{
	if (exercises.empty())
		throw std::invalid_argument("a training wills exercises");

	matter::Word uttered = utter(title);
	matter::Training trained = utter(uttered.id(), std::move(exercises));
	matter::Placement placed = spatiality().place(uttered.id(), shepherding.id());
	matter::Dating dated = temporality().date(uttered.id());

	return std::static_pointer_cast<const Training>(shepherding.inscribe(
		Birth<Testator>{*this},
		matter::Behest{std::move(uttered), std::move(placed), std::move(dated), std::move(trained)}));
}


const Shepherding& Testator::shepherding(const Novice& novice) const
{
	for (const Shepherding* place : shepherdings_) {
		if (place && place->novice().Soul::id() == novice.Soul::id())
			return *place;
	}

	throw std::invalid_argument("unknown shepherding");
}


const Shepherding& Testator::shepherd(const Shepherding& place) const
{
	if (place.testator().Soul::id() != Soul::id())
		throw std::logic_error("not the testator of this shepherding");

	for (const Shepherding* existing : shepherdings_) {
		if (existing && existing->id() == place.id())
			return *existing;
	}

	shepherdings_.push_back(&place);
	return place;
}


void Testator::release(const Shepherding& place) const
{
	const auto it = std::find(shepherdings_.begin(), shepherdings_.end(), &place);
	if (it != shepherdings_.end())
		shepherdings_.erase(it);
}


std::shared_ptr<const Supplication> Testator::supplication(const Novice& suppliant) const
{
	for (std::shared_ptr<const Supplication>& pending : supplications()) {
		if (pending->suppliant().Soul::id() == suppliant.Soul::id())
			return std::move(pending);
	}

	throw UnknownSupplication();
}


std::vector<std::shared_ptr<const Supplication>> Testator::supplications() const
{
	return Immanent<Heaven>::horizon().supplications(Soul::id());
}


void Testator::hear(const Birth<Novice> birth) const
{
	const Man& suppliant = man_of(birth.parent());
	if (!abode().dweller(suppliant))
		throw HearOnlyDweller();

	for (const Shepherding* place : shepherdings_) {
		if (place && place->novice().Soul::id() == suppliant.Soul::id())
			throw AlreadyTied();
	}
}


} // namespace will::domain
