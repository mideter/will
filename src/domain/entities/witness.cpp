#include "witness.h"

#include "entities/abode.h"
#include "ports/temporality.h"

#include <algorithm>
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

	temporality().fix(abode_->id(), Soul::id(), word);
}


std::vector<Letter> Witness::retell(const std::uint32_t limit) const
{
	if (limit == 0)
		throw std::invalid_argument("History limit must be positive");

	const std::uint32_t capped = std::min(limit, Temporality::MaxLetterLimit);
	return temporality().letters(abode_->id(), capped);
}


} // namespace will::domain
