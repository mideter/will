#include "man.h"

#include <utility>


namespace will::domain {


Man::Man(const id::Man id, Soul&& soul, Vessel&& vessel)
	: Soul(std::move(soul))
	, Vessel(std::move(vessel))
	, id_(id)
{}


} // namespace will::domain
