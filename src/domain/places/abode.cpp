#include "abode.h"

#include "relations/contemplation.h"
#include "relations/friend.h"
#include "places/room.h"
#include "refusals/room.h"
#include "men/man.h"
#include "men/witness.h"
#include "words/letter.h"
#include "words/recollection.h"
#include "matter/letter.h"
#include "horizons/space.h"
#include "properties/immanent.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <utility>


namespace will::domain {


Abode::Abode(const Birth<Man> birth, matter::Abode kept)
	: Place(kept.id())
	, host_(birth.parent())
	, outer_(Birth<Abode>{*this}, kept.gates())
	, inner_(Birth<Abode>{*this}, kept.helm())
	, mutex_(std::make_unique<std::mutex>())
{
	Immanent<Space>::present<Abode>();
}


Abode::~Abode() = default;


void Abode::admit(const Birth<Man> host, const matter::Dweller& kept)
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
		born = std::make_shared<const Acquaintance>(Birth<Abode>{*this}, man);
		break;
	case matter::Dweller::Kind::Neighbour:
		born = std::make_shared<const Neighbour>(Birth<Abode>{*this}, man);
		break;
	case matter::Dweller::Kind::Friend:
		born = std::make_shared<const Friend>(Birth<Abode>{*this}, man);
		break;
	}
	{
		std::lock_guard lock(*mutex_);
		dwellers_.insert_or_assign(&man, std::move(born));
	}

	// Of a kind his part no longer lets in, he withdraws from the room he stands in.
	man.withdraw(Birth<Abode>{*this});
}


void Abode::arrange(const Birth<Man> birth, const matter::Room& kept) const
{
	if (&birth.parent() != &host_)
		throw std::logic_error("only the host arranges his abode");

	const std::vector<std::reference_wrapper<const Room>> all = rooms();
	const auto found = std::ranges::find_if(all, [&](const Room& room) { return room.id() == kept.id(); });
	if (found == all.end())
		throw std::logic_error("the room is not of this abode");
	const Room& room = *found;
	const Part& to = part(kept.part());
	if (&room.part() != &to)
		room.part().give(Birth<Abode>{*this}, room, to);

	// Those standing in it whom its new part does not let in withdraw.
	for (const std::shared_ptr<const Acquaintance>& dweller : dwellers())
		dweller->man().withdraw(Birth<Abode>{*this});
}


const Part& Abode::part(const matter::Room::Part kept) const
{
	if (kept == matter::Room::Part::Outer)
		return outer_;
	return inner_;
}


std::vector<std::reference_wrapper<const Room>> Abode::rooms() const
{
	std::vector<std::reference_wrapper<const Room>> out = outer_.rooms();
	const std::vector<std::reference_wrapper<const Room>> inner = inner_.rooms();
	out.insert(out.end(), inner.begin(), inner.end());
	return out;
}


const Room* Abode::room(const Place& reflects) const
{
	for (const Room& room : rooms()) {
		if (&room.reflects() == &reflects)
			return &room;
	}
	return nullptr;
}


void Abode::host_only(const Contemplation& gaze, const Place& here) const
{
	if (&gaze.place() != &here)
		throw std::logic_error("this place is not what is contemplated");
	if (&gaze.who() != &host_)
		throw OnlyHostActs();
}


std::vector<std::shared_ptr<const Acquaintance>> Abode::dwellers() const
{
	std::lock_guard lock(*mutex_);

	std::vector<std::shared_ptr<const Acquaintance>> out;
	out.reserve(dwellers_.size());
	for (const auto& [man, dweller] : dwellers_)
		out.push_back(dweller);
	return out;
}


std::shared_ptr<const Acquaintance> Abode::dweller(const Man& man) const
{
	if (host_.descends_from(man, matter::Fatherhood::Line::Spirit))
		return std::make_shared<const Friend>(Birth<Abode>{*this}, man);

	std::lock_guard lock(*mutex_);
	const auto it = dwellers_.find(&man);
	if (it == dwellers_.end())
		return nullptr;

	return it->second;
}


bool Abode::dwells(const Man& man) const
{
	if (&man == &host_ || host_.descends_from(man, matter::Fatherhood::Line::Spirit))
		return true;

	std::lock_guard lock(*mutex_);
	return dwellers_.contains(&man);
}


bool Abode::leads_to(const Man&, const Place& next) const
{
	return next.within(*this) || &next == &outer_.gates();
}


bool Abode::within(const Abode& abode) const
{
	return &abode == this;
}


std::shared_ptr<const Word> Abode::receive(const Birth<Room> room, const Saying& saying) const
{
	if (&room.parent().abode() != this)
		throw std::logic_error("the room is not of this abode");

	// What is written here is its host's record: a letter.
	return host_.record(room, saying);
}


std::shared_ptr<const Letter> Abode::inscribe(Birth<Witness>, matter::Letter kept) const
{
	if (kept.placement().place() != id())
		throw std::logic_error("the letter is not placed in this abode");
	if (kept.word().author() != host_.Soul::id())
		throw std::logic_error("only the host inscribes a letter");

	auto letter = std::make_shared<const Letter>(Birth<Abode>{*this}, std::move(kept));
	enter(*recollection(), letter);

	return letter;
}


} // namespace will::domain
