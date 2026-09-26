#include "supplication.h"

#include "beings/immanents/obedience.h"
#include "beings/immanents/shepherding.h"
#include "acts/tying.h"
#include "beings/immanents/novice.h"
#include "beings/immanents/soul.h"
#include "beings/immanents/testator.h"
#include "ports/temporality.h"

#include <stdexcept>


namespace will::domain {


Supplication::Supplication(const Novice& suppliant, const Testator& addressee)
	: suppliant_(suppliant)
	, addressee_(addressee)
{
	if (suppliant.Soul::id() == addressee.Soul::id())
		throw std::invalid_argument("supplication requires distinct suppliant and addressee");
}


void Supplication::sign(const Novice& suppliant) const
{
	if (suppliant.Soul::id() != suppliant_.Soul::id())
		throw std::logic_error("only the suppliant may sign this supplication");

	addressee_.receive(Supplication{*this});
}


void Supplication::sign(const Testator& addressee) const
{
	if (addressee.Soul::id() != addressee_.Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");

	const Supplication& incoming = addressee.supplication(suppliant_);

	const Tying tying = addressee.temporality().accept(incoming);
	const Obedience& place = static_cast<const Novice&>(Soul::of(tying.novice())).keep(tying);
	addressee.keep(dynamic_cast<const Shepherding&>(place));
	addressee.drop(incoming);
}


void Supplication::reject(const Testator& addressee) const
{
	if (addressee.Soul::id() != addressee_.Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");

	const Supplication& incoming = addressee.supplication(suppliant_);

	addressee.temporality().reject(incoming);
	addressee.drop(incoming);
}


} // namespace will::domain
