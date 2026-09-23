#include "world.h"

#include "acts/obedience.h"
#include "acts/shepherding.h"
#include "acts/supplication.h"
#include "acts/tying.h"
#include "beings/novice.h"
#include "beings/soul.h"
#include "beings/testator.h"
#include "ports/temporality.h"
#include "values/soul_name.h"

#include <stdexcept>
#include <utility>
#include <vector>


namespace will::domain {


World::World(Heaven heaven, Earth earth)
	: Heaven(std::move(heaven))
	, Earth(std::move(earth))
{}


void World::awaken()
{
	for (Embodiment e : temporality().embodiments())
		(void)accept(std::move(e));

	for (Tying tying : temporality().tyings()) {
		const auto& novice = static_cast<const Novice&>(Soul::of(tying.novice()));
		const Obedience& place = novice.keep(std::move(tying));
		static_cast<const Testator&>(place.testator()).keep(
			dynamic_cast<const Shepherding&>(place));
	}

	for (const auto& [soul_id, man] : men_) {
		std::vector<Supplication> pending = temporality().pending_supplications(soul_id);
		for (Supplication& ask : pending)
			static_cast<const Testator&>(*man).receive(std::move(ask));
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
