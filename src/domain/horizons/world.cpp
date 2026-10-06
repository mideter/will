#include "dimensions/eternity.h"
#include "world.h"

#include "places/room.h"
#include "relations/contemplation.h"

#include "places/obedience.h"
#include "places/shepherding.h"
#include "relations/supplication.h"
#include "matter/embodiment.h"
#include "matter/man.h"
#include "matter/soul.h"
#include "matter/tie.h"
#include "matter/vessel.h"
#include "men/novice.h"
#include "men/soul.h"
#include "men/testator.h"
#include "men/witness.h"
#include "dimensions/spatiality.h"
#include "dimensions/temporality.h"
#include "values/soul_name.h"

#include <algorithm>
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


World::~World()
{
	// Ties are let go while both their sides still live: the testator is told
	// that his shepherding ends.
	for (auto& [soul_id, man] : men_)
		static_cast<const Novice&>(*man).obediences_.clear();
}


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
		vessels.erase(vessel);
	}

	// The bodies no soul dwells in await their birth.
	std::vector<matter::Vessel> awaiting;
	for (auto& [id, kept] : vessels)
		awaiting.push_back(std::move(kept));
	std::sort(awaiting.begin(), awaiting.end(),
			  [](const matter::Vessel& a, const matter::Vessel& b) { return a.id() < b.id(); });
	for (matter::Vessel& kept : awaiting)
		(void)await(std::move(kept));

	for (const matter::Fatherhood& kept : eternity().fatherhoods())
		living_man(kept.child()).descend(Birth<World>{*this}, living_man(kept.father()), kept);

	for (matter::Tie kept : spatiality().ties()) {
		const auto& novice = static_cast<const Novice&>(Soul::of(kept.novice()));
		const Obedience& place = novice.follow(std::move(kept));
		static_cast<const Testator&>(place.testator()).shepherd(
			dynamic_cast<const Shepherding&>(place));
	}

	// Tie rooms are recalled once the ties live; a tie bound before rooms were
	// is given its rooms now.
	for (const auto& [soul_id, man] : men_) {
		Abode& abode = man->abode();
		for (const matter::Room& kept : spatiality().rooms(abode.id())) {
			if (kept.reflects() != abode.id())
				abode.furnish(Birth<World>{*this}, kept);
		}
	}
	for (const matter::Tie& kept : spatiality().ties()) {
		const Place& tie = Place::of(kept.id());
		for (const id::Soul side : {kept.testator(), kept.novice()}) {
			Abode& abode = living_man(side).abode();
			if (!abode.room(tie))
				abode.furnish(Birth<World>{*this},
							  spatiality().furnish(abode.id(), kept.id(), matter::Room::Aspect::Words));
		}
	}

	std::unordered_map<id::Place, Man*> hosts;
	for (const auto& [soul_id, man] : men_)
		hosts.emplace(man->abode().id(), man.get());
	for (const matter::Dweller& kept : spatiality().dwellers()) {
		const auto host = hosts.find(kept.abode());
		if (host == hosts.end())
			throw std::runtime_error("a kept dweller dwells in an abode no living man keeps");

		host->second->abode().admit(Birth<World>{*this}, living_man(kept.soul()), kept);
	}

	for (const auto& [soul_id, man] : men_) {
		const auto& addressee = static_cast<const Testator&>(*man);

		for (matter::Supplication kept : temporality().supplications(soul_id))
			addressee.hear(Birth<World>{*this}, std::move(kept));
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


const Vessel& World::welcome(const DeviceToken& token)
{
	if (const auto vessel_id = id_of(token))
		return vessel(*vessel_id);

	bool first = false;
	{
		std::lock_guard lock(mutex_);
		first = men_.empty();
	}
	if (first)
		return beget(token);

	return await(temporality().form(token));
}


std::vector<std::reference_wrapper<const Unborn>> World::unborn() const
{
	std::lock_guard lock(mutex_);

	std::vector<std::reference_wrapper<const Unborn>> out;
	out.reserve(unborn_.size());
	for (const std::unique_ptr<Unborn>& body : unborn_)
		out.emplace_back(*body);
	return out;
}


const Unborn& World::unborn(const id::Vessel id) const
{
	std::lock_guard lock(mutex_);

	for (const std::unique_ptr<Unborn>& body : unborn_) {
		if (body->id() == id)
			return *body;
	}
	throw std::invalid_argument("no such unborn");
}


const Man& World::bear(const Man& midwife, const Unborn& unborn)
{
	const std::shared_ptr<const Contemplation> gaze = contemplation(midwife.Soul::id());
	const auto* room = gaze ? dynamic_cast<const BirthRoom*>(&gaze->place()) : nullptr;
	if (!room)
		throw std::logic_error("one bears only standing in a birth room");

	const Man& father = room->abode().host();
	matter::Vessel body{unborn.id(), unborn.token()};
	{
		std::lock_guard lock(mutex_);
		const auto it = std::find_if(unborn_.begin(), unborn_.end(),
									 [&](const std::unique_ptr<Unborn>& body) { return body.get() == &unborn; });
		if (it == unborn_.end())
			throw std::invalid_argument("no such unborn");
	}

	matter::Soul soul = eternity().enroll(SoulName::generate());
	const matter::Embodiment embodied = temporality().embody(soul.id(), body.id());
	const matter::Fatherhood fatherhood = eternity().father(father.Soul::id(), soul.id(), matter::Fatherhood::Line::Flesh);

	const Man& child = accept(matter::Man{std::move(soul), std::move(body), embodied});
	{
		// The man is born: his body no longer awaits.
		std::lock_guard lock(mutex_);
		std::erase_if(unborn_, [&](const std::unique_ptr<Unborn>& body) { return body.get() == &unborn; });
	}

	child.descend(Birth<World>{*this}, father, fatherhood);
	child.abode().admit(Birth<World>{*this}, father,
						spatiality().dwell(child.abode().id(), father.Soul::id(), matter::Dweller::Kind::Neighbour));
	if (!father.abode().dwells(child))
		father.abode().admit(Birth<World>{*this}, child,
							 spatiality().dwell(father.abode().id(), child.Soul::id(),
												matter::Dweller::Kind::Acquaintance));
	return child;
}


void World::choose_father(const Man& child, const Man& father)
{
	using Line = matter::Fatherhood::Line;

	if (&child == &father)
		throw std::logic_error("one is not one's own father");
	for (const Man* elder = father.father(Line::Spirit); elder; elder = elder->father(Line::Spirit)) {
		if (elder == &child)
			throw std::logic_error("one does not choose a descendant by spirit as one's father");
	}

	const matter::Fatherhood kept = eternity().father(father.Soul::id(), child.Soul::id(), Line::Spirit);
	child.descend(Birth<World>{*this}, father, kept);
}


std::vector<World::Descent> World::lineage(const Man& father) const
{
	std::vector<const Man*> men;
	{
		std::lock_guard lock(mutex_);
		men.reserve(men_.size());
		for (const auto& [soul_id, man] : men_)
			men.push_back(man.get());
	}
	std::sort(men.begin(), men.end(), [](const Man* a, const Man* b) { return a->Soul::id() < b->Soul::id(); });

	std::vector<Descent> line;
	std::vector<const Man*> generation{&father};
	while (!generation.empty()) {
		std::vector<const Man*> next;
		for (const Man* elder : generation) {
			for (const Man* man : men) {
				if (man->father(matter::Fatherhood::Line::Spirit) == elder) {
					line.push_back(Descent{*man, *elder});
					next.push_back(man);
				}
			}
		}
		generation = std::move(next);
	}
	return line;
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


std::vector<std::reference_wrapper<const Man>> World::hosts_of(const Man& dweller) const
{
	std::lock_guard lock(mutex_);

	std::vector<std::reference_wrapper<const Man>> hosts;
	for (const auto& [soul_id, man] : men_) {
		if (man && man->abode().dweller(dweller))
			hosts.emplace_back(*man);
	}
	return hosts;
}


const Man& World::beget(const DeviceToken& token)
{
	matter::Soul soul = eternity().enroll(SoulName::generate());
	matter::Vessel vessel = temporality().form(token);
	const matter::Embodiment embodied = temporality().embody(soul.id(), vessel.id());

	return accept(matter::Man{std::move(soul), std::move(vessel), embodied});
}


const Unborn& World::await(matter::Vessel kept)
{
	auto body = std::make_unique<Unborn>(Birth<World>{*this}, std::move(kept));
	const Unborn& awaiting = *body;

	std::lock_guard lock(mutex_);
	unborn_.push_back(std::move(body));
	return awaiting;
}


const Man& World::accept(matter::Man kept)
{
	const id::Soul soul_id = kept.soul().id();
	const id::Vessel vessel_id = kept.vessel().id();
	auto ptr = std::unique_ptr<Man>(std::make_unique<Testator>(Birth<World>{*this}, std::move(kept)));
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
