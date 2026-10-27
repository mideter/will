#include "outer_abode.h"

#include "men/man.h"
#include "places/room.h"
#include "refusals/room.h"
#include "relations/contemplation.h"
#include "relations/friend.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


OuterAbode::OuterAbode(const Birth<Man> birth, const matter::Abode& kept)
	: Place(kept.id())
	, host_(birth.parent())
	, gates_(Birth<OuterAbode>{*this}, kept.gates())
{}


OuterAbode::~OuterAbode() = default;


std::vector<std::reference_wrapper<const Room>> OuterAbode::outer_rooms() const
{
	std::lock_guard lock(rooms_mutex_);

	std::vector<std::reference_wrapper<const Room>> out;
	out.reserve(outer_rooms_.size());
	for (const std::unique_ptr<Room>& room : outer_rooms_)
		out.emplace_back(*room);
	return out;
}


void OuterAbode::admit(const Birth<Man> host, const matter::Dweller& kept)
{
	if (&host.parent() != &host_)
		throw std::logic_error("only the host brings one to dwell in his abode");
	if (kept.abode() != id())
		throw std::logic_error("the dweller is not of this abode");
	if (kept.soul() == host_.Soul::id())
		throw std::logic_error("the host is not a dweller of his own abode");

	const Man& man = Man::of(kept.soul());

	// Regarded anew, the dweller is born anew of his new kind.
	std::shared_ptr<const Acquaintance> born;
	switch (kept.kind()) {
	case matter::Dweller::Kind::Acquaintance:
		born = std::make_shared<const Acquaintance>(Birth<OuterAbode>{*this}, man);
		break;
	case matter::Dweller::Kind::Neighbour:
		born = std::make_shared<const Neighbour>(Birth<OuterAbode>{*this}, man);
		break;
	case matter::Dweller::Kind::Friend:
		born = std::make_shared<const Friend>(Birth<OuterAbode>{*this}, man);
		break;
	}
	{
		std::lock_guard lock(dwellers_mutex_);
		dwellers_.insert_or_assign(&man, std::move(born));
	}

	// Of a kind that no longer sees the room he stands in, he withdraws from it.
	man.withdraw(Birth<OuterAbode>{*this});
}


std::vector<std::shared_ptr<const Acquaintance>> OuterAbode::dwellers() const
{
	std::lock_guard lock(dwellers_mutex_);

	std::vector<std::shared_ptr<const Acquaintance>> out;
	out.reserve(dwellers_.size());
	for (const auto& [man, dweller] : dwellers_)
		out.push_back(dweller);
	return out;
}


std::shared_ptr<const Acquaintance> OuterAbode::dweller(const Man& man) const
{
	if (host_.descends_from(man, matter::Fatherhood::Line::Spirit))
		return std::make_shared<const Friend>(Birth<OuterAbode>{*this}, man);

	std::lock_guard lock(dwellers_mutex_);
	const auto it = dwellers_.find(&man);
	if (it == dwellers_.end())
		return nullptr;

	return it->second;
}


bool OuterAbode::dwells(const Man& man) const
{
	if (&man == &host_ || host_.descends_from(man, matter::Fatherhood::Line::Spirit))
		return true;

	std::lock_guard lock(dwellers_mutex_);
	return dwellers_.contains(&man);
}


void OuterAbode::host_only(const Contemplation& gaze, const Place& here) const
{
	if (&gaze.place() != &here)
		throw std::logic_error("this place is not what is contemplated");
	if (&gaze.who() != &host_)
		throw OnlyHostActs();
}


bool OuterAbode::leads_to(const Man&, const Place& next) const
{
	return next.within(*this) || &next == &gates_;
}


bool OuterAbode::within(const OuterAbode& abode) const
{
	return &abode == this;
}


void OuterAbode::withdraw_dwellers() const
{
	for (const std::shared_ptr<const Acquaintance>& dweller : dwellers())
		dweller->man().withdraw(Birth<OuterAbode>{*this});
}


} // namespace will::domain
