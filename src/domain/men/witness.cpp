#include "witness.h"

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
		throw std::logic_error("Witness does not dwell in this abode");

	Spirit::contemplate(abode);
}


void Witness::contemplate(const Room& room) const
{
	if (!room.dwells(*this))
		throw std::logic_error("Witness does not enter this room");

	Spirit::contemplate(room);
}


bool Witness::contemplates(const Place& place) const
{
	const std::shared_ptr<const Contemplation> gaze = contemplation();
	return gaze && &gaze->place().source() == &place.source();
}


void Witness::say(const Saying& saying) const
{
	const std::shared_ptr<const Contemplation> gaze = contemplation();
	if (!gaze)
		throw std::logic_error("Witness is asleep");

	// One writes one's own records only in one's cell.
	const auto* room = dynamic_cast<const Room*>(&gaze->place());
	if (!room || &room->abode() != &abode() || &room->reflects() != static_cast<const Place*>(&abode()))
		throw std::logic_error("one says only in one's cell");

	const Abode& place = abode();

	matter::Word uttered = utter(saying);
	matter::Placement placed = spatiality().place(uttered.id(), place.id());
	matter::Dating dated = temporality().date(uttered.id());

	place.inscribe(*gaze, matter::Letter{std::move(uttered), std::move(placed), std::move(dated)});
}


} // namespace will::domain
