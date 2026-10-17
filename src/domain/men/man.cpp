#include "man.h"
#include "refusals/room.h"
#include "refusals/abode.h"
#include "places/abode.h"
#include "refusals/refusal.h"

#include "dimensions/spatiality.h"
#include "men/man.h"
#include "men/witness.h"
#include "relations/contemplation.h"
#include "places/room.h"
#include "values/abode_name.h"

#include <optional>
#include <vector>
#include <algorithm>
#include <string>
#include <utility>


namespace will::domain {


Man::Man(Birth<World>, matter::Man kept)
	: Embodied(kept)
{
	Immanent<Heaven>::present<Soul>();
	Immanent<Earth>::present<Vessel>();

	std::optional<matter::Abode> own = spatiality().abode(Soul::id());
	if (!own)
		own = spatiality().abide(Soul::id(), AbodeName{std::string{Soul::name().text()}});

	abode_ = std::make_unique<Abode>(Birth<Man>{*this}, std::move(*own));

	// The standard rooms: Слово, Врата, Горница.
	std::vector<matter::Room> rooms = spatiality().rooms(abode_->id());
	for (const matter::Room::Aspect aspect :
		 {matter::Room::Aspect::Words, matter::Room::Aspect::Threshold, matter::Room::Aspect::Dwellers}) {
		const auto found = std::find_if(rooms.begin(), rooms.end(), [&](const matter::Room& room) {
			return room.reflects() == abode_->id() && room.aspect() == aspect;
		});
		abode_->furnish(Birth<Man>{*this},
						found != rooms.end() ? *found : spatiality().furnish(abode_->id(), abode_->id(), aspect));
	}
}


const Man& Man::of(const id::Soul id)
{
	return dynamic_cast<const Man&>(Soul::of(id));
}


void Man::admit(const Birth<Gates> gates, const Man& man) const
{
	abode_->admit(gates, man, spatiality().dwell(abode_->id(), man.Soul::id(), matter::Dweller::Kind::Acquaintance));
}


void Man::arrange(Birth<UpperRoom>, const Room& room, const matter::Room::Part part) const
{
	if (&room.abode() != abode_.get())
		throw std::logic_error("the room is not of this abode");

	abode_->arrange(Birth<Man>{*this}, spatiality().arrange(room.id(), part));
}


void Man::regard(Birth<UpperRoom>, const Man& dweller, const matter::Dweller::Kind kind) const
{
	if (abode_->ancestor(dweller))
		throw FatherBySpiritFriend();
	if (!abode_->dweller(dweller))
		throw DoesNotDwell();
	if (&dweller == father(matter::Fatherhood::Line::Flesh) && kind == matter::Dweller::Kind::Acquaintance)
		throw FatherByFleshNeighbour();

	abode_->admit(Birth<Man>{*this}, dweller, spatiality().dwell(abode_->id(), dweller.Soul::id(), kind));
}


Man::~Man() = default;


const Man* Man::father(const matter::Fatherhood::Line line) const noexcept
{
	switch (line) {
	case matter::Fatherhood::Line::Flesh:
		return father_by_flesh_.load();
	case matter::Fatherhood::Line::Spirit:
		return father_by_spirit_.load();
	}
	return nullptr;
}


void Man::descend(Birth<World>, const Man& father, const matter::Fatherhood& kept) const
{
	if (kept.child() != Soul::id() || kept.father() != father.Soul::id())
		throw std::logic_error("the fatherhood is of other souls");

	switch (kept.line()) {
	case matter::Fatherhood::Line::Flesh:
		father_by_flesh_.store(&father);
		break;
	case matter::Fatherhood::Line::Spirit:
		father_by_spirit_.store(&father);
		break;
	}
}


} // namespace will::domain
