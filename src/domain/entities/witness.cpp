#include "witness.h"

#include "entities/abode.h"

#include <stdexcept>


namespace will::domain {


Witness::Witness(Man&& man, Abode& abode) noexcept
	: Man(std::move(man))
	, abode_(&abode)
{}


void Witness::say(std::string_view body) const
{
	abode().inscribe(*this, body);
}


std::vector<Letter> Witness::hear(const std::uint32_t limit) const
{
	if (limit == 0)
		throw std::invalid_argument("Hear limit must be positive");

	return abode().letters(limit);
}


} // namespace will::domain
