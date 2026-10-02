#include "soul.h"

#include <utility>


namespace will::domain::matter {


Soul::Soul(const id::Soul id, SoulName name)
	: id_(id)
	, name_(std::move(name))
{}


} // namespace will::domain::matter
