#include "world.h"

#include <utility>


namespace will::domain {


World::World(Eternity& eternity)
	: Heaven(eternity)
{
	for (Vessel vessel : load_vessels())
		Earth::insert(std::move(vessel));

	for (Man man : load_men())
		insert(std::move(man));
}


std::optional<Man> World::find_man_by_token(const DeviceToken& token) const
{
	const std::optional<Vessel> vessel = find_vessel_by_token(token);
	if (!vessel)
		return std::nullopt;

	std::lock_guard lock(mutex_);

	const auto man_it = man_id_by_vessel_.find(vessel->id());
	if (man_it == man_id_by_vessel_.end())
		return std::nullopt;

	const auto it = men_by_id_.find(man_it->second);
	if (it == men_by_id_.end())
		return std::nullopt;

	return it->second;
}


std::optional<id::Soul> World::soul_id_for_token(const DeviceToken& token) const
{
	if (const std::optional<Man> man = find_man_by_token(token))
		return man->soul_id();

	return std::nullopt;
}


Man World::birth_man(const DeviceToken& token)
{
	ManBirth birth = Heaven::birth_man(token);
	Earth::insert(std::move(birth.vessel));
	Man man = std::move(birth.man);
	insert(man);
	return man;
}


void World::insert(Man man)
{
	std::lock_guard lock(mutex_);
	man_id_by_vessel_.insert_or_assign(man.vessel_id(), man.id());
	men_by_id_.insert_or_assign(man.id(), std::move(man));
}


} // namespace will::domain
