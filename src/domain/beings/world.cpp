#include "world.h"

#include "beings/testator.h"
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
		(void)accept(std::move(e));
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
	const id::Soul soul = eternity().enroll(SoulName::generate());
	return accept(temporality().embody(soul, token));
}


const Man& World::accept(Embodiment embodiment)
{
	const id::Man man_id = embodiment.man();
	const id::Vessel vessel_id = embodiment.vessel();
	auto ptr = std::unique_ptr<Man>(new Testator(std::move(embodiment)));
	Man& live = *ptr;

	std::lock_guard lock(mutex_);
	men_.insert_or_assign(man_id, std::move(ptr));
	man_id_by_vessel_.insert_or_assign(vessel_id, man_id);
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


} // namespace will::domain
