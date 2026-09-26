#include "soul.h"


namespace will::domain {


Soul::Soul(const id::Soul id, SoulName name)
	: id_(id)
	, name_(std::move(name))
{}


bool Soul::operator==(const Soul& other) const noexcept
{
	return id_ == other.id_ && name_ == other.name_;
}


const Soul& Soul::of(const id::Soul id)
{
	return heaven().soul(id);
}


} // namespace will::domain
