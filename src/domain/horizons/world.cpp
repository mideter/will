#include "dimensions/eternity.h"
#include "world.h"

#include "immanents/obedience.h"
#include "immanents/shepherding.h"
#include "acts/supplication.h"
#include "matter/embodiment.h"
#include "matter/man.h"
#include "matter/soul.h"
#include "matter/tie.h"
#include "matter/vessel.h"
#include "immanents/novice.h"
#include "immanents/soul.h"
#include "immanents/testator.h"
#include "immanents/witness.h"
#include "dimensions/spatiality.h"
#include "dimensions/temporality.h"
#include "values/soul_name.h"

#include <functional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>


namespace will::domain {


World::World(Eternity& eternity, Temporality& temporality, Spatiality& spatiality)
	: Heaven(eternity)
	, Earth(temporality, spatiality)
{}


void World::awaken()
{
	std::unordered_map<id::Soul, matter::Soul> souls;
	for (matter::Soul& kept : eternity().souls())
		souls.emplace(kept.id(), std::move(kept));

	std::unordered_map<id::Vessel, matter::Vessel> vessels;
	for (matter::Vessel& kept : temporality().vessels())
		vessels.emplace(kept.id(), std::move(kept));

	for (const matter::Embodiment& embodied : temporality().embodiments()) {
		const auto soul = souls.find(embodied.soul());
		const auto vessel = vessels.find(embodied.vessel());
		if (soul == souls.end() || vessel == vessels.end())
			throw std::runtime_error("a kept embodiment joins a soul or a vessel that is not kept");

		(void)accept(matter::Man{soul->second, vessel->second, embodied});
	}

	for (matter::Tie kept : spatiality().ties()) {
		const auto& novice = static_cast<const Novice&>(Soul::of(kept.novice()));
		const Obedience& place = novice.follow(std::move(kept));
		static_cast<const Testator&>(place.testator()).shepherd(
			dynamic_cast<const Shepherding&>(place));
	}

	for (const auto& [soul_id, man] : men_) {
		const auto& addressee = static_cast<const Testator&>(*man);

		for (matter::Supplication kept : temporality().supplications(soul_id)) {
			Supplication ask{std::move(kept)};
			// A bound pair's supplication was answered by the bond itself.
			if (ask.suppliant().follows(addressee))
				continue;

			addressee.receive(std::move(ask));
		}
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


const Man& World::man(const SoulName& name) const
{
	std::lock_guard lock(mutex_);

	for (const auto& [soul_id, man_ptr] : men_) {
		if (man_ptr && man_ptr->name() == name)
			return *man_ptr;
	}

	throw std::invalid_argument("unknown soul name");
}


const Man& World::beget(const DeviceToken& token)
{
	matter::Soul soul = eternity().enroll(SoulName::generate());
	const matter::Embodiment embodied = temporality().embody(soul.id(), token);

	return accept(matter::Man{std::move(soul), matter::Vessel{embodied.vessel(), token}, embodied});
}


const Man& World::accept(matter::Man kept)
{
	const id::Soul soul_id = kept.soul().id();
	const id::Vessel vessel_id = kept.vessel().id();
	auto ptr = std::unique_ptr<Man>(new Testator(std::move(kept)));
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
