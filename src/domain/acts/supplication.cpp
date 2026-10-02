#include "supplication.h"

#include "beings/immanents/obedience.h"
#include "beings/immanents/shepherding.h"
#include "acts/boundness.h"
#include "beings/immanents/novice.h"
#include "beings/immanents/soul.h"
#include "beings/immanents/testator.h"
#include "ports/spatiality.h"
#include "ports/temporality.h"

#include <stdexcept>


namespace will::domain {


Supplication::Supplication(Asking asking)
	: suppliant_(static_cast<const Novice&>(Soul::of(asking.suppliant())))
	, addressee_(static_cast<const Testator&>(Soul::of(asking.addressee())))
{}


void Supplication::sign(const Novice& suppliant) const
{
	if (suppliant.Soul::id() != suppliant_.Soul::id())
		throw std::logic_error("only the suppliant may sign this supplication");

	addressee_.receive(Supplication{*this});
}


const Shepherding& Supplication::sign(const Testator& addressee) const
{
	if (addressee.Soul::id() != addressee_.Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");

	const Supplication& incoming = addressee.supplication(suppliant_);
	const Novice& suppliant = suppliant_;

	const Boundness boundness = addressee.spatiality().bind(addressee.Soul::id(), suppliant.Soul::id());

	const Obedience& place = suppliant.follow(boundness);
	const Shepherding& shepherded = addressee.shepherd(dynamic_cast<const Shepherding&>(place));
	addressee.drop(incoming);

	return shepherded;
}


void Supplication::reject(const Testator& addressee) const
{
	if (addressee.Soul::id() != addressee_.Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");

	const Supplication& incoming = addressee.supplication(suppliant_);

	addressee.temporality().reject(suppliant_.Soul::id(), addressee_.Soul::id());
	addressee.drop(incoming);
}


} // namespace will::domain
