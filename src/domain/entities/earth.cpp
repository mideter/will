#include "earth.h"

#include "entities/abode.h"
#include "entities/vessel.h"
#include "entities/world.h"
#include "ports/temporality.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Earth* Earth::current_ = nullptr;


Earth& Earth::the()
{
	if (current_ == nullptr)
		throw std::logic_error("Earth has not been brought forth");
	return *current_;
}


Earth::Earth(Temporality& temporality)
	: temporality_(temporality)
{
	if (current_ != nullptr)
		throw std::logic_error("Only one World");

	bool saw_world_abode = false;
	for (Abode& place : temporality.abodes()) {
		const id::Abode id = place.id();
		if (id == id::Abode::global()) {
			saw_world_abode = true;
			continue; // World itself is the global abode
		}
		auto owned = std::make_unique<Abode>(std::move(place));
		Abode* raw = owned.get();
		owned_abodes_.emplace(id, std::move(owned));
		abodes_.emplace(id, raw);
	}

	if (!saw_world_abode)
		throw std::logic_error("Earth requires the world abode in Temporality");

	current_ = this;
}


Earth::Earth(Earth&& other) noexcept
	: temporality_(other.temporality_)
	, vessels_(std::move(other.vessels_))
	, id_by_token_(std::move(other.id_by_token_))
	, abodes_(std::move(other.abodes_))
	, owned_abodes_(std::move(other.owned_abodes_))
{
	if (current_ == &other)
		current_ = this;
}


Earth::~Earth()
{
	if (current_ == this)
		current_ = nullptr;
}


bool Earth::knows(const id::Vessel id) const
{
	std::lock_guard lock(mutex_);
	return vessels_.contains(id);
}


bool Earth::knows(const id::Abode id) const
{
	if (id == id::Abode::global())
		return current_ == this;

	std::lock_guard lock(mutex_);
	return abodes_.contains(id);
}


Abode& Earth::abode(const id::Abode id)
{
	if (id == id::Abode::global())
		return static_cast<Abode&>(static_cast<World&>(*this));

	std::lock_guard lock(mutex_);
	const auto it = abodes_.find(id);
	if (it == abodes_.end() || !it->second)
		throw std::logic_error("Earth does not know this abode");
	return *it->second;
}


const Abode& Earth::abode(const id::Abode id) const
{
	if (id == id::Abode::global())
		return static_cast<const Abode&>(static_cast<const World&>(*this));

	std::lock_guard lock(mutex_);
	const auto it = abodes_.find(id);
	if (it == abodes_.end() || !it->second)
		throw std::logic_error("Earth does not know this abode");
	return *it->second;
}


const Vessel& Earth::vessel(const id::Vessel id) const
{
	std::lock_guard lock(mutex_);

	const auto it = vessels_.find(id);
	if (it == vessels_.end() || !it->second)
		throw std::logic_error("Earth does not know this vessel");

	return *it->second;
}


std::optional<id::Vessel> Earth::id_of(const DeviceToken& token) const
{
	std::lock_guard lock(mutex_);

	const auto it = id_by_token_.find(token);
	if (it == id_by_token_.end())
		return std::nullopt;

	return it->second;
}


void Earth::index(const Vessel& vessel)
{
	std::lock_guard lock(mutex_);
	id_by_token_.insert_or_assign(vessel.token(), vessel.id());
	vessels_.insert_or_assign(vessel.id(), &vessel);
}


void Earth::index(Abode&& place)
{
	const id::Abode id = place.id();
	if (id == id::Abode::global())
		throw std::logic_error("World is the global abode");

	auto owned = std::make_unique<Abode>(std::move(place));
	Abode* raw = owned.get();
	std::lock_guard lock(mutex_);
	owned_abodes_.insert_or_assign(id, std::move(owned));
	abodes_.insert_or_assign(id, raw);
}


void Earth::join_abode(const id::Abode abode, const id::Man man)
{
	temporality_.join_abode(abode, man);
}


std::vector<std::pair<id::Abode, id::Man>> Earth::abode_men() const
{
	return temporality_.abode_men();
}


} // namespace will::domain
