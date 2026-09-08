#include "witness.h"

#include "entities/abode.h"


namespace will::domain {


Witness::Witness(Man&& man, Abode& abode) noexcept
	: Man(std::move(man))
	, abode_(&abode)
{}


void Witness::say(std::string_view body) const
{
	abode().inscribe(*this, body);
}


} // namespace will::domain
