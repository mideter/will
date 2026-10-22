#include "part.h"

#include "places/abode.h"
#include "relations/acquaintance.h"

#include <memory>


namespace will::domain {


Part::Part(const Birth<Abode> birth)
	: abode_(birth.parent())
{}


bool Part::admits(const Man& man) const
{
	if (&man == &abode_.host())
		return true;

	const std::shared_ptr<const Acquaintance> dweller = abode_.dweller(man);
	return dweller && lets_in(*dweller);
}


Outer::Outer(const Birth<Abode> birth)
	: Part(birth)
{}


matter::Room::Part Outer::kept() const
{
	return matter::Room::Part::Outer;
}


bool Outer::lets_in(const Acquaintance& dweller) const
{
	return dweller.enters(*this);
}


Inner::Inner(const Birth<Abode> birth)
	: Part(birth)
{}


matter::Room::Part Inner::kept() const
{
	return matter::Room::Part::Inner;
}


bool Inner::lets_in(const Acquaintance& dweller) const
{
	return dweller.enters(*this);
}


} // namespace will::domain
