#include "vessel.h"

#include <utility>


namespace will::domain {


Vessel::Vessel(matter::Vessel kept)
	: id_(kept.id())
	, token_(kept.token())
{}


bool Vessel::operator==(const Vessel& other) const noexcept
{
	return id_ == other.id_ && token_ == other.token_;
}


} // namespace will::domain
