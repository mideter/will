#include "witness.h"

#include "entities/abode.h"

#include <algorithm>
#include <stdexcept>


namespace will::domain {


Witness::Witness(Man&& man, Abode& abode, Temporality& temporality) noexcept
	: Man(std::move(man))
	, abode_(&abode)
	, temporality_(&temporality)
{}


void Witness::say(std::string_view body) const
{
	if (!abode_->dwells(*this))
		throw std::logic_error("Witness does not dwell in the observed abode");

	temporality_->fix(abode_->id(), Soul::id(), body);
}


std::vector<Letter> Witness::hear(const std::uint32_t limit) const
{
	if (limit == 0)
		throw std::invalid_argument("Hear limit must be positive");

	if (!abode_->dwells(*this))
		throw std::logic_error("Witness does not dwell in the observed abode");

	const std::uint32_t capped = std::min(limit, MaxHearLimit);
	return temporality_->letters(abode_->id(), capped);
}


} // namespace will::domain
