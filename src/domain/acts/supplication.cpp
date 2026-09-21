#include "supplication.h"

#include "acts/obedience.h"
#include "acts/shepherding.h"
#include "beings/novice.h"
#include "beings/soul.h"
#include "beings/testator.h"
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

	const Obedience created = addressee.temporality().accept(incoming);
	const Testator& place_testator = created.testator();
	const Novice& place_novice = created.novice();
	const id::Obedience oid = created.obedience_id();

	addressee.keep(Shepherding{oid, place_testator, place_novice});
	place_novice.keep(Obedience{oid, place_testator, place_novice});
	addressee.drop_supplication(suppliant_);
}


void Supplication::reject(const Testator& addressee) const
{
	if (addressee.Soul::id() != addressee_.Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");

	const Supplication& incoming = addressee.supplication(suppliant_);

	addressee.temporality().reject(incoming);
	addressee.drop_supplication(suppliant_);
}


} // namespace will::domain
