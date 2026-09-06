#include "world.h"

#include <optional>
#include <stdexcept>
#include <utility>


namespace will::domain {


World::World(Eternity& eternity, LetterRepository& letters)
	: Heaven(eternity)
	, abode_(id::Abode::global(), letters, *this)
{
	for (Man man : remember())
		accept(std::move(man));
}


Man World::find_man_by_vessel(const Vessel& vessel) const
{
	std::lock_guard lock(mutex_);

	const auto man_it = man_id_by_vessel_.find(vessel.id());
	if (man_it == man_id_by_vessel_.end())
		throw std::logic_error("Vessel has no man");

	const auto it = men_by_id_.find(man_it->second);
	if (it == men_by_id_.end() || !it->second)
		throw std::logic_error("Vessel has no man");

	return *it->second;
}


Man World::welcome(const DeviceToken& token)
{
	if (const std::optional<Vessel> vessel = find_vessel_by_token(token))
		return find_man_by_vessel(*vessel);

	return beget(token);
}


Man World::beget(const DeviceToken& token)
{
	Man man = Heaven::beget(token);
	accept(man);
	return man;
}


void World::accept(Man man)
{
	auto ptr = std::make_unique<Man>(std::move(man));
	// Man stays on the heap; moving unique_ptr does not invalidate these references.
	const Soul& soul = *ptr;
	const Vessel& vessel = *ptr;
	const id::Man man_id = ptr->id();
	const id::Vessel vessel_id = vessel.id();

	std::lock_guard lock(mutex_);
	men_by_id_.insert_or_assign(man_id, std::move(ptr));
	man_id_by_vessel_.insert_or_assign(vessel_id, man_id);
	Heaven::index(soul);
	Earth::index(vessel);
}


} // namespace will::domain
