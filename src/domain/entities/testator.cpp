#include "testator.h"

#include "entities/soul.h"
#include "ports/temporality.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Testator::Testator(const id::Man id, const id::Soul soul_id, SoulName name, const id::Vessel vessel_id,
				   DeviceToken token)
	: Executor(id, soul_id, std::move(name), vessel_id, std::move(token))
{}


Obedience Testator::accept(const Supplication& supplication) const
{
	if (supplication.addressee().id() != Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");
	if (supplication.status() != SupplicationStatus::pending)
		throw std::logic_error("supplication is not pending");

	return temporality().accept(supplication.id());
}


void Testator::refuse(const Supplication& supplication) const
{
	if (supplication.addressee().id() != Soul::id())
		throw std::logic_error("supplication is not addressed to this soul");
	if (supplication.status() != SupplicationStatus::pending)
		throw std::logic_error("supplication is not pending");

	temporality().refuse(supplication.id());
}


} // namespace will::domain
