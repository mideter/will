#include "supplication.h"
#include "refusals/tie.h"
#include "places/tie.h"
#include "refusals/refusal.h"

#include "places/abode.h"

#include "places/obedience.h"
#include "places/shepherding.h"
#include "matter/tie.h"
#include "men/man.h"
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
	: suppliant_(Man::of(kept.suppliant()))
	, addressee_(Man::of(kept.addressee()))
{}


const Shepherding& Supplication::sign(const Testator& asked) const
{
	if (asked.Soul::id() != addressee_.Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");

	const Man& addressee = addressee_;
	const Man& suppliant = suppliant_;
	const std::shared_ptr<const Supplication> incoming = addressee.supplication(suppliant);
	if (!addressee.abode().dweller(suppliant) || !suppliant.abode().dweller(addressee))
		throw TieOnlyMutualDwellers();

	const matter::Tie kept = addressee.spatiality().bind(addressee.Soul::id(), suppliant.Soul::id());
	addressee.temporality().answer(suppliant.Soul::id(), addressee.Soul::id(), matter::Answer::Form::Accepted);

	const Obedience& place = suppliant.follow(kept);
	const Shepherding& shepherded = addressee.shepherd(dynamic_cast<const Shepherding&>(place));

	// The tie is reflected in the abode of each side: Ведение and Послушание.
	constexpr auto words = matter::Room::Aspect::Words;
	addressee.abode().furnish(Birth<Supplication>{*this},
							  addressee.spatiality().furnish(addressee.abode().id(), kept.id(), words));
	suppliant.abode().furnish(Birth<Supplication>{*this},
							  addressee.spatiality().furnish(suppliant.abode().id(), kept.id(), words));

	addressee.drop(*incoming);

	return shepherded;
}


void Supplication::reject(const Testator& asked) const
{
	if (asked.Soul::id() != addressee_.Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");

	const Man& addressee = addressee_;
	const std::shared_ptr<const Supplication> incoming = addressee.supplication(suppliant_);

	addressee.temporality().answer(suppliant_.Soul::id(), addressee_.Soul::id(), matter::Answer::Form::Rejected);
	addressee.drop(*incoming);
}


} // namespace will::domain
