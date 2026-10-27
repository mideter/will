#include "abode.h"

#include "relations/contemplation.h"
#include "relations/friend.h"
#include "places/room.h"
#include "places/tie.h"
#include "words/behest.h"
#include "words/deed.h"
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
	, outer_(Birth<Abode>{*this})
	, inner_(Birth<Abode>{*this})
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
		const auto found = std::find_if(rooms_.begin(), rooms_.end(), [&](const std::unique_ptr<Room>& room) {
			const auto* cell = dynamic_cast<const Cell*>(room.get());
			return cell && cell->id() == kept.id();
		});
		if (found == rooms_.end())
			throw std::logic_error("the cell is not of this abode");
		static_cast<const Cell&>(**found).arrange(Birth<Abode>{*this}, part(kept.part()));
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
		if (&room->reflects() == &reflects && room->aspect() == kept.aspect())
			throw std::logic_error("the abode already has a room reflecting this place");
	}

	switch (kept.aspect()) {
	case matter::Room::Aspect::Words:
		rooms_.push_back(std::make_unique<Cell>(Birth<Abode>{*this}, reflects, part(kept.part()), kept));
		break;
	case matter::Room::Aspect::Threshold:
		rooms_.push_back(std::make_unique<Gates>(Birth<Abode>{*this}, kept));
		break;
	case matter::Room::Aspect::Dwellers:
		rooms_.push_back(std::make_unique<Helm>(Birth<Abode>{*this}, kept));
		break;
	}
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


const Cell* Abode::cell(const Place& reflects) const
{
	std::lock_guard lock(*mutex_);
	for (const std::unique_ptr<Room>& room : rooms_) {
		const auto* cell = dynamic_cast<const Cell*>(room.get());
		if (cell && &cell->reflects() == &reflects)
			return cell;
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


const Helm& Abode::helm() const
{
	std::lock_guard lock(*mutex_);
	for (const std::unique_ptr<Room>& room : rooms_) {
		if (const auto* helm = dynamic_cast<const Helm*>(room.get()))
			return *helm;
	}
	throw std::logic_error("the abode has no helm");
}


const Cell* Abode::cell(const std::string_view name) const
{
	std::lock_guard lock(*mutex_);
	for (const std::unique_ptr<Room>& room : rooms_) {
		const auto* cell = dynamic_cast<const Cell*>(room.get());
		if (cell && cell->name() == name)
			return cell;
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


bool Abode::shows(const Man&, const Word&) const
{
	return false;
}


std::vector<std::shared_ptr<const Behest>> Abode::owed() const
{
	return unfulfilled(true);
}


std::vector<std::shared_ptr<const Behest>> Abode::awaited() const
{
	return unfulfilled(false);
}


std::vector<std::shared_ptr<const Behest>> Abode::unfulfilled(const bool host_is_novice) const
{
	std::vector<std::shared_ptr<const Behest>> waiting;
	for (const Room& room : rooms()) {
		const auto* tie = dynamic_cast<const Tie*>(&room.reflects());
		if (!tie || (tie->novice().Soul::id() == host_.Soul::id()) != host_is_novice)
			continue;

		for (std::shared_ptr<const Behest>& behest : tie->behests(Birth<Abode>{*this})) {
			if (!behest->deed())
				waiting.push_back(std::move(behest));
		}
	}
	return waiting;
}


std::vector<std::shared_ptr<const Behest>> Abode::unreviewed() const
{
	std::vector<std::shared_ptr<const Behest>> waiting;
	for (const Room& room : rooms()) {
		const auto* tie = dynamic_cast<const Tie*>(&room.reflects());
		if (!tie || tie->testator().Soul::id() != host_.Soul::id())
			continue;

		for (std::shared_ptr<const Behest>& behest : tie->behests(Birth<Abode>{*this})) {
			const std::shared_ptr<const Deed> deed = behest->deed();
			if (deed && !deed->review())
				waiting.push_back(std::move(behest));
		}
	}
	return waiting;
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


bool Abode::leads_to(const Place& next) const
{
	return next.within(*this) || &next == &gates();
}


bool Abode::within(const Abode& abode) const
{
	return &abode == this;
}


std::shared_ptr<const Word> Abode::receive(const Birth<Cell> cell, const Saying& saying) const
{
	if (&cell.parent().abode() != this)
		throw std::logic_error("the cell is not of this abode");

	// What is written here is its host's record: a letter.
	return host_.record(cell, saying);
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
