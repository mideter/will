#include "witness.h"
#include "values/refusal.h"

#include "matter/dating.h"
#include "matter/letter.h"
#include "matter/placement.h"
#include "places/room.h"
#include "words/letter.h"
#include "matter/word.h"
#include "places/abode.h"
#include "dimensions/spatiality.h"
#include "dimensions/temporality.h"

#include <memory>
#include <stdexcept>
#include <utility>


namespace will::domain {


Witness::Witness(matter::Man kept)
	: Man(std::move(kept))
{}


void Witness::wake() const
{
	contemplate(abode());
}


void Witness::sleep() const
{
	cease();
}


void Witness::contemplate(const Abode& abode) const
{
	if (!abode.dwells(*this))
		throw Refusal(Refusal::Reason::DoesNotDwell, "Witness does not dwell in this abode");

	Spirit::contemplate(abode);
}


void Witness::contemplate(const Room& room) const
{
	if (!room.dwells(*this))
		throw Refusal(Refusal::Reason::DoesNotEnterRoom, "Witness does not enter this room");

	Spirit::contemplate(room);
}


bool Witness::contemplates(const Place& place) const
{
	const std::shared_ptr<const Contemplation> gaze = contemplation();
	return gaze && &gaze->place().source() == &place.source();
}


void Witness::say(const Saying& saying) const
{
	(void)record(saying);
}


std::shared_ptr<const Letter> Witness::record(const Saying& saying) const
{
	const std::shared_ptr<const Contemplation> gaze = contemplation();
	if (!gaze)
		throw std::logic_error("Witness is asleep");

	// One writes one's own records only in one's word: the room of one's abode that
	// shows its words — the gates and the upper room reflect it too, but show none.
	const auto* room = dynamic_cast<const Room*>(&gaze->place());
	if (!room || &room->abode() != &abode() || &room->reflects() != static_cast<const Place*>(&abode())
		|| room->aspect() != matter::Room::Aspect::Words)
		throw Refusal(Refusal::Reason::SayOnlyInWord, "one writes only in one's own word or in a tie one wills in");

	const Abode& place = abode();

	matter::Word uttered = utter(saying);
	matter::Placement placed = spatiality().place(uttered.id(), place.id());
	matter::Dating dated = temporality().date(uttered.id());

	return place.inscribe(*gaze, matter::Letter{std::move(uttered), std::move(placed), std::move(dated)});
}


} // namespace will::domain
