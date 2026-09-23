#include "witness.h"

#include "acts/inscription.h"
#include "ports/temporality.h"
#include "values/abode_name.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>


namespace will::domain {


Witness::Witness(Embodiment embodiment)
	: Man(std::move(embodiment))
	, abode_(std::make_unique<Abode>(
		  id::Abode{Soul::id().value()},
		  AbodeName{std::string{Soul::name().text()}}))
{
	abode_->present();
	abode_->admit(*this);
	temporality().keep(abode_->abode_id(), abode_->name());
	temporality().join_abode(abode_->abode_id(), Soul::id());
}


void Witness::say(const Word& word) const
{
	if (!abode_->dwells(*this))
		throw std::logic_error("Witness does not dwell in the observed abode");

	temporality().inscribe(abode_->id(), Soul::id(), word);
}


std::vector<Letter> Witness::retell(const std::uint32_t limit) const
{
	if (limit == 0)
		throw std::invalid_argument("History limit must be positive");

	const std::uint32_t capped = std::min(limit, Temporality::MaxLetterLimit);
	std::vector<Inscription> rows = temporality().inscriptions(abode_->id(), capped);
	std::vector<Letter> living;
	living.reserve(rows.size());

	for (Inscription& row : rows) {
		if (heaven().knows(row.author()))
			living.push_back(Letter{std::move(row)});
	}

	return living;
}


} // namespace will::domain
