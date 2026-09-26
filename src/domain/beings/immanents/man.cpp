#include "man.h"

#include "ports/spatiality.h"
#include "values/abode_name.h"

#include <string>
#include <utility>


namespace will::domain {


Man::Man(Embodiment embodiment)
	: Soul(embodiment.soul(), embodiment.name())
	, Vessel(embodiment.vessel(), embodiment.token())
{
	Immanent<Heaven>::present<Soul>();
	Immanent<Earth>::present<Vessel>();

	abode_ = std::make_unique<Abode>(spatiality().abide(
		Soul::id(), AbodeName{std::string{Soul::name().text()}}));
	abode_->admit(*this);
}


Man::~Man() = default;


} // namespace will::domain
