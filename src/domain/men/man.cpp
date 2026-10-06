#include "man.h"

#include "dimensions/spatiality.h"
#include "places/room.h"
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

	// The cell, the room of one's own records, reflects the abode itself.
	std::optional<matter::Room> cell;
	for (matter::Room& kept : spatiality().rooms(abode_->id())) {
		if (kept.reflects() == abode_->id())
			cell = std::move(kept);
	}
	if (!cell)
		cell = spatiality().furnish(abode_->id(), abode_->id());

	abode_->furnish(Birth<Man>{*this}, *cell);
}


void Man::admit(const Man& man) const
{
	if (abode_->dwells(man))
		throw std::logic_error("he already dwells in this abode");

	abode_->admit(Birth<Man>{*this}, man,
				  spatiality().dwell(abode_->id(), man.Soul::id(), matter::Dweller::Kind::Acquaintance));
}


void Man::arrange(const Room& room, const matter::Room::Part part) const
{
	if (&room.abode() != abode_.get())
		throw std::logic_error("the room is not of this abode");

	abode_->arrange(Birth<Man>{*this}, spatiality().arrange(room.id(), part));
}


void Man::regard(const Man& dweller, const matter::Dweller::Kind kind) const
{
	if (!abode_->dweller(dweller))
		throw std::logic_error("he does not dwell in this abode");

	abode_->admit(Birth<Man>{*this}, dweller, spatiality().dwell(abode_->id(), dweller.Soul::id(), kind));
}


Man::~Man() = default;


} // namespace will::domain
