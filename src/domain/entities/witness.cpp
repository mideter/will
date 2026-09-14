#include "witness.h"

#include "entities/abode.h"

#include <stdexcept>


namespace will::domain {


Witness::Witness(Man&& man, Abode& abode) noexcept
	: Man(std::move(man))
	, abode_(&abode)
{}


void Witness::say(const Word& word) const
{
	if (!abode_->dwells(*this))
		throw std::logic_error("Witness does not dwell in the observed abode");

	fix(abode_->id(), Soul::id(), word);
}


} // namespace will::domain
