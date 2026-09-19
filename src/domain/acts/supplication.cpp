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


Supplication::Supplication(const Soul& suppliant, const Soul& testator,
						   const SupplicationStatus status, Timestamp created_at)
	: suppliant_(suppliant)
	, testator_(testator)
	, status_(status)
	, created_at_(std::move(created_at))
{
	if (suppliant.id() == testator.id())
		throw std::invalid_argument("supplication requires distinct suppliant and testator");
}


void Supplication::entrust(const Executor& executor) const
{
	if (executor.Soul::id() != suppliant_.id())
		throw std::logic_error("only the suppliant may entrust this supplication");
	if (status_ != SupplicationStatus::pending)
		throw std::logic_error("supplication is not pending");

	static_cast<const Testator&>(testator_).receive(Supplication{*this});
}


const Obedience& Supplication::consent(const Testator& testator) const
{
	if (testator.Soul::id() != testator_.id())
		throw std::logic_error("supplication is not addressed to this soul");

	const Supplication& incoming =
		testator.supplication(static_cast<const Executor&>(suppliant_));
	if (incoming.status() != SupplicationStatus::pending)
		throw std::logic_error("supplication is not pending");

	const Obedience created = testator.temporality().accept(incoming);
	const Testator& place_testator = created.testator();
	const Executor& place_executor = created.executor();
	const id::Obedience oid = created.obedience_id();

	testator.keep(Shepherding{oid, place_testator, place_executor});
	const Obedience& obedience =
		place_executor.keep(Obedience{oid, place_testator, place_executor});
	testator.drop_supplication(static_cast<const Executor&>(suppliant_));
	return obedience;
}


void Supplication::dismiss(const Testator& testator) const
{
	if (testator.Soul::id() != testator_.id())
		throw std::logic_error("supplication is not addressed to this soul");

	const Supplication& incoming =
		testator.supplication(static_cast<const Executor&>(suppliant_));
	if (incoming.status() != SupplicationStatus::pending)
		throw std::logic_error("supplication is not pending");

	testator.temporality().refuse(incoming);
	testator.drop_supplication(static_cast<const Executor&>(suppliant_));
}


} // namespace will::domain
