#include "supplication.h"

#include "acts/obedience.h"
#include "acts/shepherding.h"
#include "beings/executor.h"
#include "beings/soul.h"
#include "beings/testator.h"
#include "ports/temporality.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Supplication::Supplication(const Executor& suppliant, const Testator& testator,
						   const SupplicationStatus status, Timestamp created_at)
	: suppliant_(suppliant)
	, testator_(testator)
	, status_(status)
	, created_at_(std::move(created_at))
{
	if (suppliant.Soul::id() == testator.Soul::id())
		throw std::invalid_argument("supplication requires distinct suppliant and testator");
}


void Supplication::sign(const Executor& executor) const
{
	if (executor.Soul::id() != suppliant_.Soul::id())
		throw std::logic_error("only the suppliant may sign this supplication");
	if (status_ != SupplicationStatus::pending)
		throw std::logic_error("supplication is not pending");

	testator_.receive(Supplication{*this});
}


void Supplication::sign(const Testator& testator) const
{
	if (testator.Soul::id() != testator_.Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");

	const Supplication& incoming = testator.supplication(suppliant_);
	if (incoming.status() != SupplicationStatus::pending)
		throw std::logic_error("supplication is not pending");

	const Obedience created = testator.temporality().accept(incoming);
	const Testator& place_testator = created.testator();
	const Executor& place_executor = created.executor();
	const id::Obedience oid = created.obedience_id();

	testator.keep(Shepherding{oid, place_testator, place_executor});
	place_executor.keep(Obedience{oid, place_testator, place_executor});
	testator.drop_supplication(suppliant_);
}


void Supplication::reject(const Testator& testator) const
{
	if (testator.Soul::id() != testator_.Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");

	const Supplication& incoming = testator.supplication(suppliant_);
	if (incoming.status() != SupplicationStatus::pending)
		throw std::logic_error("supplication is not pending");

	testator.temporality().refuse(incoming);
	testator.drop_supplication(suppliant_);
}


} // namespace will::domain
