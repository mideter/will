#include "witness.h"


namespace will::domain {


Witness::Witness(Man&& man, Abode& abode) noexcept
	: Man(std::move(man))
	, abode_(&abode)
{}


} // namespace will::domain
