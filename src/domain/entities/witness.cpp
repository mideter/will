#include "witness.h"

#include "entities/abode.h"

#include <stdexcept>
#include <vector>


namespace will::domain {


Witness::Witness(Man&& man, Abode& abode, Temporality& temporality) noexcept
	: Man(std::move(man))
	, abode_(&abode)
	, temporality_(&temporality)
{}


void Witness::say(const Word& word) const
{
	if (!abode_->dwells(*this))
		throw std::logic_error("Witness does not dwell in the observed abode");

	temporality_->fix(abode_->id(), Soul::id(), word);
}


Word Witness::hear() const
{
	if (!abode_->dwells(*this))
		throw std::logic_error("Witness does not dwell in the observed abode");

	const std::vector<Letter> rows = temporality_->letters(abode_->id(), 1);
	if (rows.empty())
		throw std::logic_error("Nothing to hear");

	return rows.front();
}


} // namespace will::domain
