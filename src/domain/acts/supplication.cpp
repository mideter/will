#include "supplication.h"

#include "acts/obedience.h"
#include "acts/shepherding.h"
#include "beings/novice.h"
#include "beings/soul.h"
#include "beings/testator.h"
#include "ports/temporality.h"

#include <stdexcept>


namespace will::domain {


Supplication::Supplication(const Novice& suppliant, const Testator& testator)
	: suppliant_(suppliant)
	, testator_(testator)
{
	if (suppliant.Soul::id() == testator.Soul::id())
		throw std::invalid_argument("supplication requires distinct suppliant and testator");
}


void Supplication::sign(const Novice& novice) const
{
	if (novice.Soul::id() != suppliant_.Soul::id())
		throw std::logic_error("only the suppliant may sign this supplication");

	testator_.receive(Supplication{*this});
}


void Supplication::sign(const Testator& testator) const
{
	if (testator.Soul::id() != testator_.Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");

	const Supplication& incoming = testator.supplication(suppliant_);

	const Obedience created = testator.temporality().accept(incoming);
	const Testator& place_testator = created.testator();
	const Novice& place_novice = created.novice();
	const id::Obedience oid = created.obedience_id();

	testator.keep(Shepherding{oid, place_testator, place_novice});
	place_novice.keep(Obedience{oid, place_testator, place_novice});
	testator.drop_supplication(suppliant_);
}


void Supplication::reject(const Testator& testator) const
{
	if (testator.Soul::id() != testator_.Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");

	const Supplication& incoming = testator.supplication(suppliant_);

	testator.temporality().refuse(incoming);
	testator.drop_supplication(suppliant_);
}


} // namespace will::domain
