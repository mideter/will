#include "man.h"

#include <utility>


namespace will::domain::matter {


Man::Man(const id::Soul soul, SoulName name, const id::Vessel vessel, DeviceToken token)
	: soul_(soul)
	, name_(std::move(name))
	, vessel_(vessel)
	, token_(std::move(token))
{}


} // namespace will::domain::matter
