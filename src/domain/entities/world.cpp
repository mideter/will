#include "world.h"

#include "entities/witness.h"
#include "identity/abode.h"
#include "ports/temporality.h"
#include "values/soul_name.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


World::World(Heaven heaven, Earth earth)
	: Heaven(std::move(heaven))
	, Earth(std::move(earth))
{
	for (Man man : eternity().men())
		(void)accept(std::move(man));

	restore_dwellers();
}


const Man& World::man(const Vessel& vessel) const
{
	std::lock_guard lock(mutex_);

	const auto man_it = man_id_by_vessel_.find(vessel.id());
	if (man_it == man_id_by_vessel_.end())
		throw std::logic_error("Vessel has no man");

	const auto it = men_.find(man_it->second);
	if (it == men_.end() || !it->second)
		throw std::logic_error("Vessel has no man");

	return *it->second;
}


const Man& World::welcome(const DeviceToken& token)
{
	if (const auto vessel_id = id_of(token))
		return man(vessel(*vessel_id));

	return beget(token);
}


const Man& World::beget(const DeviceToken& token)
{
	return accept(eternity().enroll(token, SoulName::generate()));
}


const Man& World::accept(Man&& man)
{
	Abode& place = abode();
	auto ptr = std::make_unique<Witness>(std::move(man), place);
	// Witness stays on the heap; moving unique_ptr does not invalidate these references.
	Man& live = *ptr;
	const Soul& soul = live;
	const Vessel& vessel = live;
	const id::Man man_id = live.id();
	const id::Vessel vessel_id = vessel.id();

	std::lock_guard lock(mutex_);
	men_.insert_or_assign(man_id, std::move(ptr));
	man_id_by_vessel_.insert_or_assign(vessel_id, man_id);
	Heaven::index(soul);
	Earth::index(vessel);
	place.admit(live);
	temporality().join_abode(place.id(), man_id);
	return live;
}


const Man& World::living_man(const id::Man id) const
{
	std::lock_guard lock(mutex_);

	const auto it = men_.find(id);
	if (it == men_.end() || !it->second)
		throw std::logic_error("Unknown man");

	return *it->second;
}


void World::restore_dwellers()
{
	for (const auto& [abode_id, man_id] : temporality().abode_men()) {
		if (!knows(abode_id))
			continue;

		try {
			abode(abode_id).admit(living_man(man_id));
		} catch (const std::logic_error&) {
			continue;
		}
	}
}


} // namespace will::domain
