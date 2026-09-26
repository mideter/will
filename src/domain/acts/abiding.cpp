#include "abiding.h"

#include <utility>


namespace will::domain {


Abiding::Abiding(const id::Abode id, AbodeName name)
	: id_(id)
	, name_(std::move(name))
{}


} // namespace will::domain
