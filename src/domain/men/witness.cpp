#include "witness.h"
#include "men/man.h"
#include "refusals/room.h"
#include "refusals/abode.h"
#include "refusals/place.h"
#include "places/place.h"

#include "matter/dating.h"
#include "matter/letter.h"
#include "matter/placement.h"
#include "places/room.h"
#include "places/gates.h"
#include "places/helm.h"
#include "places/shepherding.h"
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
	Spirit::contemplate(abode());
}


void Witness::sleep() const
{
	repose();
}


void Witness::contemplate(const Abode& abode) const
{
	if (!abode.dwells(man_of(*this)))
		throw DoesNotDwell();

	pass(abode);
	Spirit::contemplate(abode);
}


void Witness::contemplate(const Room& room) const
{
	enter(room);

	// The testator looking at his tie sees what was done in it.
	const auto* shepherding = dynamic_cast<const Shepherding*>(&room.reflects());
	if (shepherding && shepherding->testator().Soul::id() == man_of(*this).Soul::id())
		man_of(*this).review(Birth<Witness>{*this}, *shepherding);
}


void Witness::contemplate(const Gates& gates) const
{
	enter(gates);
}


void Witness::contemplate(const Helm& helm) const
{
	enter(helm);
}


void Witness::enter(const Place& place) const
{
	if (!place.dwells(man_of(*this)))
		throw DoesNotEnterRoom();

	pass(place);
	Spirit::contemplate(place);
}


void Witness::pass(const Place& next) const
{
	// Asleep, one is at home: waking, one sees one's own abode.
	const std::shared_ptr<const Contemplation> gaze = contemplation();
	const Place& here = gaze ? gaze->place() : abode();
	if (!here.leads_to(next))
		throw PassThroughGates();
}


bool Witness::contemplates(const Place& place) const
{
	const std::shared_ptr<const Contemplation> gaze = contemplation();
	return gaze && &gaze->place() == &place;
}


void Witness::withdraw(const Birth<Abode> abode) const
{
	const std::shared_ptr<const Contemplation> gaze = contemplation();
	const auto* room = gaze ? dynamic_cast<const Room*>(&gaze->place()) : nullptr;
	if (!room || &room->abode() != &abode.parent() || room->dwells(man_of(*this)))
		return;

	Spirit::contemplate(abode.parent());
}


std::shared_ptr<const Letter> Witness::record(const Birth<Room> room, const Saying& saying) const
{
	if (&room.parent().abode() != &abode())
		throw std::logic_error("the room is not of this abode");

	const Abode& place = abode();
	matter::Word uttered = utter(saying);
	matter::Placement placed = spatiality().place(uttered.id(), place.id());
	matter::Dating dated = temporality().date(uttered.id());
	return place.inscribe(Birth<Witness>{*this}, matter::Letter{std::move(uttered), std::move(placed), std::move(dated)});
}


} // namespace will::domain
