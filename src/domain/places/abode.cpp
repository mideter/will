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
	, name_(kept.name())
	, outer_(Birth<Abode>{*this}, kept.gates())
	, inner_(Birth<Abode>{*this}, kept.helm())
	, mutex_(std::make_unique<std::mutex>())
{
	Immanent<Space>::present<Abode>();
}


Abode::~Abode() = default;


void Abode::admit(const Birth<Man> birth, const Man& man, const matter::Dweller& kept)
{
	if (&birth.parent() != &host_)
		throw std::logic_error("only the host admits into his abode");

	admit(man, kept);
	man.withdraw(Birth<Abode>{*this});
}


void Abode::admit(Birth<World>, const Man& man, const matter::Dweller& kept)
{
	admit(man, kept);
}


void Abode::admit(const Birth<Gates> birth, const Man& man, const matter::Dweller& kept)
{
	if (&birth.parent().abode() != this)
		throw std::logic_error("one is let in only at the gates of this abode");
	if (kept.kind() != matter::Dweller::Kind::Acquaintance)
		throw std::logic_error("one let in at the gates is an acquaintance");

	admit(man, kept);
}


void Abode::admit(const Man& man, const matter::Dweller& kept)
{
	if (kept.abode() != id())
		throw std::logic_error("the dweller is not of this abode");
	if (kept.soul() != man.Soul::id())
		throw std::logic_error("the dweller matter is of another soul");
	if (&man == &host_)
		throw std::logic_error("the host is not a dweller of his own abode");

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

	std::lock_guard lock(*mutex_);
	dwellers_.insert_or_assign(&man, std::move(born));
}


const Room& Abode::furnish(const Birth<Man> birth, const matter::Room& kept)
{
	if (&birth.parent() != &host_)
		throw std::logic_error("only the host furnishes his abode");

	return furnish(kept);
}


const Room& Abode::furnish(Birth<World>, const matter::Room& kept)
{
	return furnish(kept);
}


const Room& Abode::furnish(Birth<Supplication>, const matter::Room& kept)
{
	return furnish(kept);
}


void Abode::arrange(const Birth<Man> birth, const matter::Room& kept) const
{
	if (&birth.parent() != &host_)
		throw std::logic_error("only the host arranges his abode");

	{
		std::lock_guard lock(*mutex_);
		const auto found = std::find_if(rooms_.begin(), rooms_.end(),
										[&](const std::unique_ptr<Room>& room) { return room->id() == kept.id(); });
		if (found == rooms_.end())
			throw std::logic_error("the room is not of this abode");
		(*found)->arrange(Birth<Abode>{*this}, part(kept.part()));
	}
	// Those standing in it whom its new part does not let in withdraw.
	for (const std::shared_ptr<const Acquaintance>& dweller : dwellers())
		dweller->man().withdraw(Birth<Abode>{*this});
}


const Room& Abode::furnish(const matter::Room& kept)
{
	if (kept.abode() != id())
		throw std::logic_error("the room is not of this abode");

	const Place& reflects = kept.reflects() == id() ? static_cast<const Place&>(*this) : Place::of(kept.reflects());

	std::lock_guard lock(*mutex_);
	for (const std::unique_ptr<Room>& room : rooms_) {
		if (&room->reflects() == &reflects)
			throw std::logic_error("the abode already has a room reflecting this place");
	}

	rooms_.push_back(std::make_unique<Room>(Birth<Abode>{*this}, reflects, part(kept.part()), kept));
	return *rooms_.back();
}


const Part& Abode::part(const matter::Room::Part kept) const
{
	if (kept == matter::Room::Part::Outer)
		return outer_;
	return inner_;
}


std::vector<std::reference_wrapper<const Room>> Abode::rooms() const
{
	std::lock_guard lock(*mutex_);

	std::vector<std::reference_wrapper<const Room>> out;
	out.reserve(rooms_.size());
	for (const std::unique_ptr<Room>& room : rooms_)
		out.emplace_back(*room);
	return out;
}


const Room* Abode::room(const Place& reflects) const
{
	std::lock_guard lock(*mutex_);
	for (const std::unique_ptr<Room>& room : rooms_) {
		if (&room->reflects() == &reflects)
			return room.get();
	}
	return nullptr;
}


const Room* Abode::room(const std::string_view name) const
{
	std::lock_guard lock(*mutex_);
	for (const std::unique_ptr<Room>& room : rooms_) {
		if (room->name() == name)
			return room.get();
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
	if (ancestor(man))
		return std::make_shared<const Friend>(Birth<Abode>{*this}, man);

	std::lock_guard lock(*mutex_);
	const auto it = dwellers_.find(&man);
	if (it == dwellers_.end())
		return nullptr;

	return it->second;
}


bool Abode::ancestor(const Man& man) const
{
	// The line is walked no further than there are steps in it: a choice that
	// would close it into a ring is refused, yet a broken one would not hang here.
	std::size_t steps = 0;
	for (const Man* father = host_.father(matter::Fatherhood::Line::Spirit); father && steps < 4096;
		 father = father->father(matter::Fatherhood::Line::Spirit), ++steps) {
		if (father == &man)
			return true;
	}
	return false;
}


bool Abode::dwells(const Man& man) const
{
	if (&man == &host_ || ancestor(man))
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
