#include "place.h"

#include "beings/space.h"

#include <stdexcept>


namespace will::domain {


Place::Place()
	: id_(id::Place::global())
{
	throw std::logic_error("Place default ctor is only for virtual-base faces");
}


Place::Place(const id::Place id) noexcept
	: id_(id)
{}


Place::Place(Place&& other) noexcept
	: id_(other.id_)
{}


Place& Place::operator=(Place&& other) noexcept
{
	if (this == &other)
		return *this;

	id_ = other.id_;

	return *this;
}


const Place& Place::of(const id::Place id)
{
	return Space::the().place(id);
}


void Place::present() const
{
	Space::the().present(*this);
}


} // namespace will::domain
