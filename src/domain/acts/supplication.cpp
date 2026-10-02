#include "supplication.h"

#include "beings/immanents/obedience.h"
#include "beings/immanents/shepherding.h"
#include "matter/tie.h"
#include "beings/immanents/novice.h"
#include "beings/immanents/soul.h"
#include "beings/immanents/testator.h"
#include "dimensions/spatiality.h"
#include "dimensions/temporality.h"

#include <stdexcept>


namespace will::domain {


Supplication::Supplication(matter::Supplication kept)
	: suppliant_(static_cast<const Novice&>(Soul::of(kept.suppliant())))
	, addressee_(static_cast<const Testator&>(Soul::of(kept.addressee())))
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

	const matter::Tie kept = addressee.spatiality().bind(addressee.Soul::id(), suppliant.Soul::id());

	const Obedience& place = suppliant.follow(kept);
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
