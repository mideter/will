#include "abode.h"

#include "relations/contemplation.h"
#include "relations/friend.h"
#include "places/room.h"
#include "places/tie.h"
#include "men/novice.h"
#include "words/behest.h"
#include "words/deed.h"
#include "men/man.h"
#include "men/witness.h"
#include "words/letter.h"
#include "words/recollection.h"
#include "matter/letter.h"
#include "horizons/space.h"
#include "properties/immanent.h"

#include <stdexcept>
#include <unordered_set>
#include <utility>


namespace will::domain {


Abode::Abode(const Birth<Man> birth, matter::Abode kept)
	: Place(kept.id())
	, host_(birth.parent())
	, name_(kept.name())
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
}


void Abode::admit(Birth<World>, const Man& man, const matter::Dweller& kept)
{
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

	std::lock_guard lock(*mutex_);
	for (const std::unique_ptr<Room>& room : rooms_) {
		if (room->id() == kept.id()) {
			room->arrange(Birth<Abode>{*this}, kept);
			return;
		}
	}
	throw std::logic_error("the room is not of this abode");
}


const Room& Abode::furnish(const matter::Room& kept)
{
	if (kept.abode() != id())
		throw std::logic_error("the room is not of this abode");

	const Place& reflects = kept.reflects() == id() ? static_cast<const Place&>(*this) : Place::of(kept.reflects());

	std::lock_guard lock(*mutex_);
	for (const std::unique_ptr<Room>& room : rooms_) {
		if (&room->reflects() == &reflects && room->aspect() == kept.aspect())
			throw std::logic_error("the abode already has a room reflecting this place");
	}

	switch (kept.aspect()) {
	case matter::Room::Aspect::Words:
		rooms_.push_back(std::make_unique<Room>(Birth<Abode>{*this}, reflects, kept));
		break;
	case matter::Room::Aspect::Threshold:
		rooms_.push_back(std::make_unique<Gates>(Birth<Abode>{*this}, kept));
		break;
	case matter::Room::Aspect::Dwellers:
		rooms_.push_back(std::make_unique<UpperRoom>(Birth<Abode>{*this}, kept));
		break;
	case matter::Room::Aspect::Birth:
		rooms_.push_back(std::make_unique<BirthRoom>(Birth<Abode>{*this}, kept));
		break;
	}
	return *rooms_.back();
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
		if (&room->reflects() == &reflects && room->aspect() == matter::Room::Aspect::Words)
			return room.get();
	}
	return nullptr;
}


const Gates& Abode::gates() const
{
	std::lock_guard lock(*mutex_);
	for (const std::unique_ptr<Room>& room : rooms_) {
		if (const auto* gates = dynamic_cast<const Gates*>(room.get()))
			return *gates;
	}
	throw std::logic_error("the abode has no gates");
}


const UpperRoom& Abode::upper_room() const
{
	std::lock_guard lock(*mutex_);
	for (const std::unique_ptr<Room>& room : rooms_) {
		if (const auto* upper_room = dynamic_cast<const UpperRoom*>(room.get()))
			return *upper_room;
	}
	throw std::logic_error("the abode has no upper room");
}


const BirthRoom& Abode::birth_room() const
{
	std::lock_guard lock(*mutex_);
	for (const std::unique_ptr<Room>& room : rooms_) {
		if (const auto* birth_room = dynamic_cast<const BirthRoom*>(room.get()))
			return *birth_room;
	}
	throw std::logic_error("the abode has no birth room");
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
	std::lock_guard lock(*mutex_);
	const auto it = dwellers_.find(&man);
	if (it == dwellers_.end())
		return nullptr;

	return it->second;
}


bool Abode::shows(const Man&, const Word&) const
{
	return false;
}


std::vector<std::shared_ptr<const Behest>> Abode::outstanding() const
{
	const auto& asker = dynamic_cast<const Novice&>(host_);

	std::vector<std::shared_ptr<const Behest>> waiting;
	for (const Room& room : rooms()) {
		const auto* tie = dynamic_cast<const Tie*>(&room.reflects());
		if (!tie)
			continue;

		std::unordered_set<id::Word> fulfilled;
		for (const std::shared_ptr<const Deed>& deed : tie->deeds(asker))
			fulfilled.insert(deed->behest());
		for (std::shared_ptr<const Behest>& behest : tie->behests(asker)) {
			if (!fulfilled.contains(behest->id()))
				waiting.push_back(std::move(behest));
		}
	}
	return waiting;
}


bool Abode::dwells(const Man& man) const
{
	if (&man == &host_)
		return true;

	std::lock_guard lock(*mutex_);
	return dwellers_.contains(&man);
}


std::shared_ptr<const Letter> Abode::inscribe(const Contemplation& gaze, matter::Letter kept) const
{
	if (&gaze.place().source() != this)
		throw std::logic_error("this abode is not what is contemplated");
	if (kept.placement().place() != id())
		throw std::logic_error("the letter is not placed in this abode");
	if (kept.word().author() != gaze.who().Soul::id())
		throw std::logic_error("only its author inscribes a letter");
	if (kept.word().author() != host_.Soul::id())
		throw std::logic_error("only the host writes in his abode");

	auto letter = std::make_shared<const Letter>(Birth<Abode>{*this}, std::move(kept));
	enter(*recollection(gaze), letter);

	return letter;
}


} // namespace will::domain
