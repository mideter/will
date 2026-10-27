#include "part.h"

#include "places/abode.h"
#include "men/man.h"
#include "relations/acquaintance.h"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <utility>


namespace will::domain {


Part::Part(const Birth<Abode> birth)
	: abode_(birth.parent())
{}


bool Part::admits(const Man& man) const
{
	if (&man == &abode_.host())
		return true;

	const std::shared_ptr<const Acquaintance> dweller = abode_.dweller(man);
	return dweller && lets_in(*dweller);
}


std::vector<std::reference_wrapper<const Room>> Part::rooms() const
{
	std::lock_guard lock(mutex_);

	std::vector<std::reference_wrapper<const Room>> out;
	out.reserve(rooms_.size());
	for (const std::unique_ptr<Room>& room : rooms_)
		out.emplace_back(*room);
	return out;
}


bool Part::holds(const Room& room) const
{
	std::lock_guard lock(mutex_);
	return std::ranges::any_of(rooms_, [&](const std::unique_ptr<Room>& held) { return held.get() == &room; });
}


void Part::recall(const Birth<Man> host, const matter::Room& kept) const
{
	if (&host.parent() != &abode_.host())
		throw std::logic_error("only the host recalls his own room");

	lay(kept);
}


void Part::recall(Birth<World>, const matter::Room& kept) const
{
	lay(kept);
}


const Room& Part::lay(const matter::Room& kept) const
{
	if (kept.abode() != abode_.id() || kept.part() != this->kept())
		throw std::logic_error("a room lies in the part of the abode it is kept in");

	const Place& reflects = kept.reflects() == abode_.id() ? static_cast<const Place&>(abode_) : Place::of(kept.reflects());
	if (abode_.room(reflects))
		throw std::logic_error("the abode already has a room reflecting this place");

	std::lock_guard lock(mutex_);
	rooms_.push_back(std::make_unique<Room>(Birth<Part>{*this}, reflects, kept));
	return *rooms_.back();
}


void Part::give(const Birth<Abode> birth, const Room& room, const Part& to) const
{
	if (&birth.parent() != &abode_ || &to.abode() != &abode_)
		throw std::logic_error("only its abode moves a room, between its own parts");

	// Both held at once: the room is never seen in neither part, nor in both.
	std::scoped_lock lock(mutex_, to.mutex_);
	const auto found = std::ranges::find_if(rooms_, [&](const std::unique_ptr<Room>& held) { return held.get() == &room; });
	if (found == rooms_.end())
		throw std::logic_error("the room is not in this part");
	to.rooms_.push_back(std::move(*found));
	rooms_.erase(found);
}


Outer::Outer(const Birth<Abode> birth, const id::Place gates)
	: Part(birth)
	, gates_(Birth<Outer>{*this}, gates)
{}


matter::Room::Part Outer::kept() const
{
	return matter::Room::Part::Outer;
}


bool Outer::lets_in(const Acquaintance& dweller) const
{
	return dweller.enters(*this);
}


Inner::Inner(const Birth<Abode> birth, const id::Place helm)
	: Part(birth)
	, helm_(Birth<Inner>{*this}, helm)
{}


const Room& Inner::furnish(const Birth<Man> host, const matter::Room& kept) const
{
	if (&host.parent() != &abode().host())
		throw std::logic_error("only the host furnishes his abode");

	return lay(kept);
}


const Room& Inner::furnish(Birth<Supplication>, const matter::Room& kept) const
{
	return lay(kept);
}


matter::Room::Part Inner::kept() const
{
	return matter::Room::Part::Inner;
}


bool Inner::lets_in(const Acquaintance& dweller) const
{
	return dweller.enters(*this);
}


} // namespace will::domain
