#include "world.h"

#include "entities/witness.h"
#include "values/abode_name.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


World::World(Temporality& temporality)
	: Heaven(temporality)
	, abode_(id::Abode::global(), AbodeName::global(), temporality, *this)
{
	for (Man man : remember())
		(void)accept(std::move(man));
}


const Man& World::man(const Vessel& vessel) const
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


const Man& World::welcome(const DeviceToken& token)
{
	if (const auto vessel_id = id_of(token))
		return man(vessel(*vessel_id));

	return beget(token);
}


const Man& World::beget(const DeviceToken& token)
{
	return accept(Heaven::beget(token));
}


const Man& World::accept(Man&& man)
{
	auto ptr = std::make_unique<Witness>(std::move(man), abode_);
	// Witness stays on the heap; moving unique_ptr does not invalidate these references.
	Man& live = *ptr;
	const Soul& soul = live;
	const Vessel& vessel = live;
	const id::Man man_id = live.id();
	const id::Vessel vessel_id = vessel.id();

	std::lock_guard lock(mutex_);
	men_by_id_.insert_or_assign(man_id, std::move(ptr));
	man_id_by_vessel_.insert_or_assign(vessel_id, man_id);
	Heaven::index(soul);
	Earth::index(vessel);
	abode_.admit(live);
	return live;
}


} // namespace will::domain
