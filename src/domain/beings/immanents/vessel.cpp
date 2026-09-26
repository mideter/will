#include "vessel.h"

#include <utility>


namespace will::domain {


Vessel::Vessel(const id::Vessel id, DeviceToken token)
	: id_(id)
	, token_(std::move(token))
{}


bool Vessel::operator==(const Vessel& other) const noexcept
{
	return id_ == other.id_ && token_ == other.token_;
}


} // namespace will::domain
