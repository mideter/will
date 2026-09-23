#include "space.h"

#include "beings/place.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Space* Space::current_ = nullptr;


Space& Space::the()
{
	if (current_ == nullptr)
		throw std::logic_error("Space has not been brought forth");

	return *current_;
}


Space::Space()
{
	if (current_ != nullptr)
		throw std::logic_error("Only one Space");

	current_ = this;
}


Space::Space(Space&& other) noexcept
	: places_(std::move(other.places_))
{
	if (current_ == &other)
		current_ = this;
}


Space::~Space()
{
	if (current_ == this)
		current_ = nullptr;
}


bool Space::knows(const id::Place id) const
{
	std::lock_guard lock(mutex_);
	return places_.contains(id);
}


const Place& Space::place(const id::Place id) const
{
	std::lock_guard lock(mutex_);

	const auto it = places_.find(id);
	if (it == places_.end() || !it->second)
		throw std::logic_error("Space does not know this place");

	return *it->second;
}


void Space::present(const Place& place)
{
	std::lock_guard lock(mutex_);
	places_.insert_or_assign(place.id(), &place);
}


} // namespace will::domain
