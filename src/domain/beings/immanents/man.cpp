#include "man.h"

#include "dimensions/spatiality.h"
#include "values/abode_name.h"

#include <optional>
#include <string>
#include <utility>


namespace will::domain {


Man::Man(matter::Man kept)
	: Soul(kept.soul())
	, Vessel(kept.vessel())
{
	Immanent<Heaven>::present<Soul>();
	Immanent<Earth>::present<Vessel>();

	std::optional<matter::Abode> own = spatiality().abode(Soul::id());
	if (!own)
		own = spatiality().abide(Soul::id(), AbodeName{std::string{Soul::name().text()}});

	abode_ = std::make_unique<Abode>(std::move(*own));
	abode_->admit(*this);
}


Man::~Man() = default;


} // namespace will::domain
