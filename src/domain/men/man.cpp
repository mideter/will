#include "man.h"

#include "dimensions/spatiality.h"
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

	// The standard rooms reflect the abode itself: the cell its words, the gates
	// its threshold, the upper room its dwellers, the birth room the unborn.
	std::vector<matter::Room> rooms = spatiality().rooms(abode_->id());
	for (const matter::Room::Aspect aspect :
		 {matter::Room::Aspect::Words, matter::Room::Aspect::Threshold, matter::Room::Aspect::Dwellers,
		  matter::Room::Aspect::Birth}) {
		const auto found = std::find_if(rooms.begin(), rooms.end(), [&](const matter::Room& room) {
			return room.reflects() == abode_->id() && room.aspect() == aspect;
		});
		abode_->furnish(Birth<Man>{*this},
						found != rooms.end() ? *found : spatiality().furnish(abode_->id(), abode_->id(), aspect));
	}
}


void Man::admit(const Man& man) const
{
	// One lets in at the gates one stands in and keeps — one's own, or another's
	// whose kind lets one keep them; the one let in becomes the host's acquaintance.
	const std::shared_ptr<const Contemplation> gaze = contemplation();
	const auto* gates = gaze ? dynamic_cast<const Gates*>(&gaze->place()) : nullptr;
	if (!gates)
		throw std::logic_error("one admits only standing in gates");
	if (!gates->keeps(*this))
		throw std::logic_error("one keeps the gates only by one's kind");
	if (gates->abode().dwells(man))
		throw std::logic_error("he already dwells in this abode");
	if (!static_cast<const Witness&>(man).contemplates(*gates))
		throw std::logic_error("he does not stand at the gates");

	gates->admit(Birth<Man>{*this}, man,
				 spatiality().dwell(gates->abode().id(), man.Soul::id(), matter::Dweller::Kind::Acquaintance));
}


void Man::arrange(const Room& room, const matter::Room::Part part) const
{
	if (&room.abode() != abode_.get())
		throw std::logic_error("the room is not of this abode");
	if (!static_cast<const Witness&>(*this).contemplates(abode_->upper_room()))
		throw std::logic_error("one arranges rooms only in one's upper room");

	abode_->arrange(Birth<Man>{*this}, spatiality().arrange(room.id(), part));
}


void Man::regard(const Man& dweller, const matter::Dweller::Kind kind) const
{
	if (abode_->ancestor(dweller))
		throw std::logic_error("the father by spirit is ever a friend");
	if (!abode_->dweller(dweller))
		throw std::logic_error("he does not dwell in this abode");
	if (!static_cast<const Witness&>(*this).contemplates(abode_->upper_room()))
		throw std::logic_error("one regards dwellers only in one's upper room");
	if (&dweller == father(matter::Fatherhood::Line::Flesh) && kind == matter::Dweller::Kind::Acquaintance)
		throw std::logic_error("the father by flesh is ever at least a neighbour");

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
