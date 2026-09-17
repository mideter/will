#include "world.h"

#include "entities/testator.h"
#include "entities/witness.h"
#include "ports/temporality.h"
#include "values/soul_name.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


World::World(Heaven heaven, Earth earth)
	: Heaven(std::move(heaven))
	, Earth(std::move(earth))
{}


void World::awaken()
{
	for (Embodiment e : temporality().embodiments())
		(void)accept(e.man, e.soul, std::move(e.name), e.vessel, std::move(e.token));

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
	const Soul soul = eternity().enroll(SoulName::generate());
	Embodiment e = temporality().embody(soul.id(), token);
	return accept(e.man, e.soul, std::move(e.name), e.vessel, std::move(e.token));
}


const Man& World::accept(const id::Man man_id, const id::Soul soul_id, SoulName name,
						 const id::Vessel vessel_id, DeviceToken token)
{
	auto ptr = std::unique_ptr<Man>(new Testator(man_id, soul_id, std::move(name), vessel_id, std::move(token)));
	Man& live = *ptr;
	const id::Vessel live_vessel = static_cast<const Vessel&>(live).id();

	std::lock_guard lock(mutex_);
	men_.insert_or_assign(man_id, std::move(ptr));
	man_id_by_vessel_.insert_or_assign(live_vessel, man_id);
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
	for (const auto& [abode_id, man_id] : abode_men()) {
		try {
			const Man& host = living_man(id::Man{abode_id.value()});
			static_cast<const Witness&>(host).abode().admit(living_man(man_id));
		} catch (const std::logic_error&) {
			continue;
		}
	}
}


} // namespace will::domain
