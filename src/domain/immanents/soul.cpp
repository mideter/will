#include "soul.h"


namespace will::domain {


Soul::Soul(matter::Soul kept)
	: id_(kept.id())
	, name_(kept.name())
{}


bool Soul::operator==(const Soul& other) const noexcept
{
	return id_ == other.id_ && name_ == other.name_;
}


const Soul& Soul::of(const id::Soul id)
{
	return Spirit::soul(id);
}


} // namespace will::domain
