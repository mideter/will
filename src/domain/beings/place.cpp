#include "place.h"

#include "beings/space.h"

#include <stdexcept>


namespace will::domain {


Place::Place()
	: id_(id::Place{1})
{
	throw std::logic_error("Place default ctor is only for virtual-base faces");
}


Place::Place(const id::Place id) noexcept
	: id_(id)
{}


const Place& Place::of(const id::Place id)
{
	return Space::the().place(id);
}


} // namespace will::domain
