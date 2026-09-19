#include "world.h"

#include "acts/shepherding.h"
#include "beings/executor.h"
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

	for (const Obedience& place : temporality().obediences()) {
		const Soul& testator_soul = place.testator();
		const Soul& executor_soul = place.executor();
		static_cast<const Executor&>(executor_soul)
			.keep(Obedience{place.obedience_id(), testator_soul, executor_soul, place.living()});
		static_cast<const Testator&>(testator_soul)
			.keep(Shepherding{place.obedience_id(), testator_soul, executor_soul, place.living()});
	}
}


const Man& World::man(const Vessel& vessel) const
{
	std::lock_guard lock(mutex_);

	const auto soul_it = soul_id_by_vessel_.find(vessel.id());
	if (soul_it == soul_id_by_vessel_.end())
		throw std::logic_error("Vessel has no man");

	const auto it = men_.find(soul_it->second);
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
	const id::Soul soul_id = embodiment.soul();
	const id::Vessel vessel_id = embodiment.vessel();
	auto ptr = std::unique_ptr<Man>(new Testator(std::move(embodiment)));
	Man& live = *ptr;

	std::lock_guard lock(mutex_);
	men_.insert_or_assign(soul_id, std::move(ptr));
	soul_id_by_vessel_.insert_or_assign(vessel_id, soul_id);
	return live;
}


const Man& World::living_man(const id::Soul id) const
{
	std::lock_guard lock(mutex_);

	const auto it = men_.find(id);
	if (it == men_.end() || !it->second)
		throw std::logic_error("Unknown man");

	return *it->second;
}


} // namespace will::domain
