#include "place.h"


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


} // namespace will::domain
