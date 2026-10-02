#include "man.h"

#include "ports/spatiality.h"
#include "values/abode_name.h"

#include <string>
#include <utility>


namespace will::domain {


Man::Man(matter::Man kept)
	: Soul(kept.soul(), kept.name())
	, Vessel(kept.vessel(), kept.token())
{
	Immanent<Heaven>::present<Soul>();
	Immanent<Earth>::present<Vessel>();

	abode_ = std::make_unique<Abode>(spatiality().abide(
		Soul::id(), AbodeName{std::string{Soul::name().text()}}));
	abode_->admit(*this);
}


Man::~Man() = default;


} // namespace will::domain
