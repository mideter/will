#include "man.h"

#include "acts/abiding.h"
#include "beings/space.h"
#include "ports/eternity.h"
#include "ports/spatiality.h"
#include "values/abode_name.h"

#include <optional>
#include <string>
#include <utility>


namespace will::domain {


Man::Man(Embodiment embodiment)
	: Soul(embodiment.soul(), embodiment.name())
	, Vessel(embodiment.vessel(), embodiment.token())
{
	Immanent<Heaven>::present<Soul>();
	Immanent<Earth>::present<Vessel>();

	if (std::optional<Abiding> abiding = spatiality().abode_of(Soul::id())) {
		abode_ = std::make_unique<Abode>(std::move(*abiding));
	} else {
		abode_ = std::make_unique<Abode>(
			id::Abode{eternity().space().point()},
			AbodeName{std::string{Soul::name().text()}});
		spatiality().keep(abode_->abode_id(), abode_->name());
		spatiality().join_abode(abode_->abode_id(), Soul::id());
	}

	abode_->admit(*this);
}


Man::~Man() = default;


} // namespace will::domain
