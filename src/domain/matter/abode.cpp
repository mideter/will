#include "abode.h"

#include <utility>


namespace will::domain::matter {


Abode::Abode(const id::Place id, AbodeName name)
	: id_(id)
	, name_(std::move(name))
{}


} // namespace will::domain::matter
