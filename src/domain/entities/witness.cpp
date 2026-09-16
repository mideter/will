#include "witness.h"

#include "ports/temporality.h"
#include "values/abode_name.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>


namespace will::domain {


Witness::Witness(Man&& man)
	: Man(std::move(man))
	, abode_(std::make_unique<Abode>(
		  id::Abode{id().value()},
		  AbodeName{std::string{Soul::name().text()}}))
{
	abode_->admit(*this);
	temporality().keep(abode_->abode_id(), abode_->name());
	temporality().join_abode(abode_->abode_id(), id());
}


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
