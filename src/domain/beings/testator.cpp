#include "testator.h"

#include "beings/soul.h"
#include "ports/temporality.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Testator::Testator(Embodiment embodiment)
	: Executor(std::move(embodiment))
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


Deed Testator::will(const Obedience& obedience, const Word& word) const
{
	if (obedience.testator().id() != Soul::id())
		throw std::logic_error("not the testator of this obedience");

	return temporality().bequeath(obedience, static_cast<const Soul&>(*this), word);
}


} // namespace will::domain
