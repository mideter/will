#include "gates.h"
#include "refusals/room.h"
#include "refusals/abode.h"

#include "horizons/heaven.h"
#include "horizons/space.h"
#include "men/man.h"
#include "places/abode.h"
#include "places/part.h"
#include "relations/contemplation.h"
#include "relations/knock.h"

#include <algorithm>
#include <stdexcept>


namespace will::domain {


Gates::Gates(const Birth<Outer> birth, const id::Place id)
	: Place(id)
	, outer_(birth.parent())
{
	Immanent<Space>::present<Place>();
}


const Abode& Gates::abode() const noexcept
{
	return outer_.abode();
}


std::string Gates::name() const
{
	return "Врата";
}


bool Gates::dwells(const Man&) const
{
	return true;
}


bool Gates::leads_to(const Place& next) const
{
	return next.within(abode()) || dynamic_cast<const Gates*>(&next);
}


bool Gates::within(const Abode&) const
{
	return false;
}


bool Gates::keeps(const Man& man) const
{
	return &man == &abode().host();
}


bool Gates::open() const
{
	for (const Man& man : Immanent<Heaven>::horizon().contemplating(*this)) {
		if (keeps(man))
			return true;
	}
	return false;
}


void Gates::knock(const Contemplation& gaze) const
{
	if (&gaze.place() != this)
		throw std::logic_error("these gates are not what is contemplated");
	if (abode().dwells(gaze.who()))
		throw AlreadyDwells();
	if (dynamic_cast<const Knock*>(&gaze))
		throw AlreadyKnocks();

	Immanent<Heaven>::horizon().knock(Birth<Gates>{*this}, gaze);
}


std::vector<std::reference_wrapper<const Man>> Gates::knocking() const
{
	// One let in while he knocks waits no longer.
	std::vector<std::reference_wrapper<const Man>> waiting;
	for (const Man& man : Immanent<Heaven>::horizon().knocking(*this)) {
		if (!abode().dwells(man))
			waiting.emplace_back(man);
	}
	return waiting;
}


void Gates::admit(const Contemplation& gaze, const Man& man) const
{
	abode().host_only(gaze, *this);
	if (abode().dwells(man))
		throw AlreadyDwells();
	const auto waiting = knocking();
	if (std::ranges::none_of(waiting, [&](const Man& knocker) { return &knocker == &man; }))
		throw DoesNotKnock();

	abode().host().admit(Birth<Gates>{*this}, man);
}


const Man& Gates::bear(const Contemplation& gaze, const Unborn& unborn) const
{
	abode().host_only(gaze, *this);

	return Immanent<Heaven>::horizon().bear(Birth<Gates>{*this}, unborn);
}


} // namespace will::domain
