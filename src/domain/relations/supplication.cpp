#include "supplication.h"

#include "places/obedience.h"
#include "places/shepherding.h"
#include "matter/tie.h"
#include "men/novice.h"
#include "men/soul.h"
#include "men/testator.h"
#include "dimensions/spatiality.h"
#include "dimensions/temporality.h"

#include <stdexcept>


namespace will::domain {


Supplication::Supplication(Birth<Novice>, matter::Supplication kept)
	: Supplication(std::move(kept))
{}


Supplication::Supplication(Birth<World>, matter::Supplication kept)
	: Supplication(std::move(kept))
{}


Supplication::Supplication(matter::Supplication kept)
	: suppliant_(static_cast<const Novice&>(Soul::of(kept.suppliant())))
	, addressee_(static_cast<const Testator&>(Soul::of(kept.addressee())))
{}


const Shepherding& Supplication::sign(const Testator& addressee) const
{
	if (addressee.Soul::id() != addressee_.Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");

	const std::shared_ptr<const Supplication> incoming = addressee.supplication(suppliant_);
	const Novice& suppliant = suppliant_;

	const matter::Tie kept = addressee.spatiality().bind(addressee.Soul::id(), suppliant.Soul::id());
	addressee.temporality().answer(suppliant.Soul::id(), addressee.Soul::id(), matter::Answer::Form::Accepted);

	const Obedience& place = suppliant.follow(kept);
	const Shepherding& shepherded = addressee.shepherd(dynamic_cast<const Shepherding&>(place));
	addressee.drop(*incoming);

	return shepherded;
}


void Supplication::reject(const Testator& addressee) const
{
	if (addressee.Soul::id() != addressee_.Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");

	const std::shared_ptr<const Supplication> incoming = addressee.supplication(suppliant_);

	addressee.temporality().answer(suppliant_.Soul::id(), addressee_.Soul::id(), matter::Answer::Form::Rejected);
	addressee.drop(*incoming);
}


} // namespace will::domain
