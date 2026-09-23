#include "place.h"

#include "beings/space.h"


namespace will::domain {


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
