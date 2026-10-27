#include "helm.h"
#include "refusals/abode.h"

#include "horizons/space.h"
#include "men/man.h"
#include "places/abode.h"
#include "relations/acquaintance.h"
#include "relations/contemplation.h"


namespace will::domain {


Helm::Helm(const Birth<Abode> birth, const id::Place id)
	: Place(id)
	, abode_(birth.parent())
{
	Immanent<Space>::present<Place>();
}


bool Helm::leads_to(const Man& who, const Place& next) const
{
	return abode().leads_to(who, next);
}


bool Helm::within(const OuterAbode& abode) const
{
	return &abode == &abode_;
}


void Helm::regard(const Contemplation& gaze, const Man& dweller, const matter::Dweller::Kind kind) const
{
	abode().host_only(gaze, *this);

	abode().host().regard(Birth<Helm>{*this}, dweller, kind);
}


void Helm::supplicate(const Contemplation& gaze, const Man& addressee) const
{
	abode().host_only(gaze, *this);
	const std::shared_ptr<const Acquaintance> dweller = abode().dweller(addressee);
	if (!dweller)
		throw DoesNotDwell();

	abode().host().supplicate(Birth<Helm>{*this}, *dweller);
}


const Shepherding& Helm::accept(const Contemplation& gaze, const Supplication& ask) const
{
	abode().host_only(gaze, *this);

	return abode().host().accept(Birth<Helm>{*this}, ask);
}


void Helm::reject(const Contemplation& gaze, const Supplication& ask) const
{
	abode().host_only(gaze, *this);

	abode().host().reject(Birth<Helm>{*this}, ask);
}


void Helm::arrange(const Contemplation& gaze, const Room& room, const matter::Room::Part to) const
{
	abode().host_only(gaze, *this);

	abode().host().arrange(Birth<Helm>{*this}, room, to);
}


} // namespace will::domain
