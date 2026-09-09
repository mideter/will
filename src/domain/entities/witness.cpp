#include "witness.h"

#include "entities/abode.h"
#include "entities/letter.h"

#include <stdexcept>
#include <vector>


namespace will::domain {


Witness::Witness(Man&& man, Abode& abode, Earth& earth) noexcept
	: Man(std::move(man))
	, abode_(&abode)
	, earth_(&earth)
{}


void Witness::say(const Word& word) const
{
	if (!abode_->dwells(*this))
		throw std::logic_error("Witness does not dwell in the observed abode");

	earth_->fix(abode_->id(), Soul::id(), word);
}


Word Witness::hear() const
{
	if (!abode_->dwells(*this))
		throw std::logic_error("Witness does not dwell in the observed abode");

	const std::vector<Letter> rows = earth_->letters(abode_->id(), 1);
	if (rows.empty())
		throw std::logic_error("Nothing to hear");

	return rows.front();
}


} // namespace will::domain
