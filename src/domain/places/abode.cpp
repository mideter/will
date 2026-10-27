#include "abode.h"

#include "men/man.h"
#include "places/room.h"
#include "relations/acquaintance.h"
#include "words/letter.h"
#include "words/recollection.h"
#include "horizons/space.h"
#include "properties/immanent.h"

#include <algorithm>
#include <stdexcept>
#include <utility>


namespace will::domain {


Abode::Abode(const Birth<Man> birth, matter::Abode kept)
	: OuterAbode(birth, kept)
	, helm_(Birth<Abode>{*this}, kept.helm())
{
	Immanent<Space>::present<Abode>();
}


Abode::~Abode() = default;


std::vector<std::reference_wrapper<const Room>> Abode::inner_rooms() const
{
	std::lock_guard lock(rooms_mutex_);

	std::vector<std::reference_wrapper<const Room>> out;
	out.reserve(inner_rooms_.size());
	for (const std::unique_ptr<Room>& room : inner_rooms_)
		out.emplace_back(*room);
	return out;
}


std::vector<std::reference_wrapper<const Room>> Abode::rooms() const
{
	std::vector<std::reference_wrapper<const Room>> out = outer_rooms();
	const std::vector<std::reference_wrapper<const Room>> inner = inner_rooms();
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


bool Abode::inward(const Room& room) const
{
	std::lock_guard lock(rooms_mutex_);
	return std::ranges::any_of(inner_rooms_, [&](const std::unique_ptr<Room>& held) { return held.get() == &room; });
}


bool Abode::shows(const Man& man, const Room& room) const
{
	if (&man == &host())
		return true;

	const std::shared_ptr<const Acquaintance> seer = dweller(man);
	if (!seer)
		return false;

	const OuterAbode& outer = *this;
	return inward(room) ? seer->sees(*this) : seer->sees(outer);
}


const Room& Abode::furnish(const Birth<Man> host, const matter::Room& kept) const
{
	if (&host.parent() != &this->host())
		throw std::logic_error("only the host furnishes his abode");
	if (kept.part() != matter::Room::Part::Inner)
		throw std::logic_error("a new room is furnished inward");

	return lay(kept);
}


const Room& Abode::furnish(Birth<Supplication>, const matter::Room& kept) const
{
	if (kept.part() != matter::Room::Part::Inner)
		throw std::logic_error("a new room is furnished inward");

	return lay(kept);
}


void Abode::recall(const Birth<Man> host, const matter::Room& kept) const
{
	if (&host.parent() != &this->host())
		throw std::logic_error("only the host recalls his own room");

	lay(kept);
}


void Abode::recall(Birth<World>, const matter::Room& kept) const
{
	lay(kept);
}


const Room& Abode::lay(const matter::Room& kept) const
{
	if (kept.abode() != id())
		throw std::logic_error("the room is not of this abode");

	const Place& reflects = kept.reflects() == id() ? static_cast<const Place&>(*this) : Place::of(kept.reflects());
	if (room(reflects))
		throw std::logic_error("the abode already has a room reflecting this place");

	auto born = std::make_unique<Room>(Birth<Abode>{*this}, reflects, kept);
	std::lock_guard lock(rooms_mutex_);
	auto& into = kept.part() == matter::Room::Part::Inner ? inner_rooms_ : outer_rooms_;
	into.push_back(std::move(born));
	return *into.back();
}


void Abode::arrange(const Birth<Man> host, const matter::Room& kept) const
{
	if (&host.parent() != &this->host())
		throw std::logic_error("only the host arranges his abode");

	{
		std::lock_guard lock(rooms_mutex_);
		auto& to = kept.part() == matter::Room::Part::Inner ? inner_rooms_ : outer_rooms_;
		auto& from = &to == &inner_rooms_ ? outer_rooms_ : inner_rooms_;
		const auto found = std::ranges::find_if(from, [&](const std::unique_ptr<Room>& held) { return held->id() == kept.id(); });
		if (found != from.end()) {
			to.push_back(std::move(*found));
			from.erase(found);
		} else if (std::ranges::none_of(to, [&](const std::unique_ptr<Room>& held) { return held->id() == kept.id(); })) {
			throw std::logic_error("the room is not of this abode");
		}
	}

	// Those standing in it whom their kind no longer lets see it withdraw.
	withdraw_dwellers();
}


std::shared_ptr<const Word> Abode::receive(const Birth<Room> room, const Saying& saying) const
{
	if (&room.parent().abode() != this)
		throw std::logic_error("the room is not of this abode");

	// What is written here is its host's record: a letter.
	return host().record(room, saying);
}


std::shared_ptr<const Letter> Abode::inscribe(Birth<Witness>, matter::Letter kept) const
{
	if (kept.placement().place() != id())
		throw std::logic_error("the letter is not placed in this abode");
	if (kept.word().author() != host().Soul::id())
		throw std::logic_error("only the host inscribes a letter");

	auto letter = std::make_shared<const Letter>(Birth<Abode>{*this}, std::move(kept));
	enter(*recollection(), letter);

	return letter;
}


} // namespace will::domain
