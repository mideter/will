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

	abode_ = std::make_unique<Abode>(Birth<Man>{*this}, std::move(*own));
}


void Man::admit(const Man& man) const
{
	if (abode_->dwells(man))
		throw std::logic_error("he already dwells in this abode");

	abode_->admit(Birth<Man>{*this}, man,
				  spatiality().dwell(abode_->id(), man.Soul::id(), matter::Dweller::Kind::Acquaintance));
}


void Man::regard(const Man& dweller, const matter::Dweller::Kind kind) const
{
	if (!abode_->kind(dweller))
		throw std::logic_error("he does not dwell in this abode");

	abode_->admit(Birth<Man>{*this}, dweller, spatiality().dwell(abode_->id(), dweller.Soul::id(), kind));
}


Man::~Man() = default;


} // namespace will::domain
