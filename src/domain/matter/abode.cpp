#include "abode.h"

#include <utility>


namespace will::domain::matter {


Abode::Abode(const id::Place id, AbodeName name, const id::Place gates, const id::Place helm)
	: id_(id)
	, name_(std::move(name))
	, gates_(gates)
	, helm_(helm)
{}


} // namespace will::domain::matter
