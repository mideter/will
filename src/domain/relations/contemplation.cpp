#include "contemplation.h"

#include "men/man.h"
#include "places/room.h"
#include "places/gates.h"
#include "places/helm.h"
#include "refusals/place.h"
#include "words/recollection.h"


namespace will::domain {


Contemplation::Contemplation(Birth<Heaven>, const Man& who, const Place& place)
	: who_(who)
	, place_(place)
{
	// Only through a room are words seen: beholding it keeps them living.
	if (const auto* room = dynamic_cast<const Room*>(&place))
		recollection_ = room->recollection(Birth<Contemplation>{*this});
}


const Room& Contemplation::room() const
{
	const auto* room = dynamic_cast<const Room*>(&place_);
	if (!room)
		throw StandInRoom();
	return *room;
}


const Helm& Contemplation::helm() const
{
	const auto* helm = dynamic_cast<const Helm*>(&place_);
	if (!helm)
		throw StandAtHelm();
	return *helm;
}


const Gates& Contemplation::gates() const
{
	const auto* gates = dynamic_cast<const Gates*>(&place_);
	if (!gates)
		throw StandAtGates();
	return *gates;
}


bool Contemplation::writable() const
{
	const auto* room = dynamic_cast<const Room*>(&place_);
	return room && room->writable_by(who_);
}


} // namespace will::domain
