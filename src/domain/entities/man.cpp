#include "man.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Man::Man(const id::Man id, Soul&& soul, Vessel&& vessel)
	: Soul(std::move(soul))
	, Vessel(std::move(vessel))
	, id_(id)
{}


void Man::say(std::string_view) const
{
	throw std::logic_error("Man outside the living world cannot say");
}


} // namespace will::domain
