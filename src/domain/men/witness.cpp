#include "witness.h"
#include "men/man.h"
#include "refusals/room.h"
#include "refusals/abode.h"
#include "places/place.h"

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
	if (!abode.dwells(man_of(*this)))
		throw DoesNotDwell();

	Spirit::contemplate(abode);
}


void Witness::contemplate(const Room& room) const
{
	if (!room.dwells(man_of(*this)))
		throw DoesNotEnterRoom();

	Spirit::contemplate(room);
}


bool Witness::contemplates(const Place& place) const
{
	const std::shared_ptr<const Contemplation> gaze = contemplation();
	return gaze && &gaze->place().source() == &place.source();
}


std::shared_ptr<const Letter> Witness::record(const Birth<Cell> cell, const Saying& saying) const
{
	if (&cell.parent().abode() != &abode())
		throw std::logic_error("the cell is not of this abode");

	const Abode& place = abode();
	matter::Word uttered = utter(saying);
	matter::Placement placed = spatiality().place(uttered.id(), place.id());
	matter::Dating dated = temporality().date(uttered.id());
	return place.inscribe(Birth<Witness>{*this}, matter::Letter{std::move(uttered), std::move(placed), std::move(dated)});
}


} // namespace will::domain
