#include "earth.h"

#include "entities/abode.h"
#include "entities/vessel.h"
#include "ports/temporality.h"

#include <stdexcept>


namespace will::domain {


Temporality* Earth::temporality_ = nullptr;
std::mutex Earth::mutex_;
std::unordered_map<id::Vessel, const Vessel*> Earth::vessels_;
std::unordered_map<DeviceToken, id::Vessel> Earth::id_by_token_;
std::unordered_map<id::Abode, Abode*> Earth::abodes_;
std::unordered_map<id::Abode, std::unique_ptr<Abode>> Earth::owned_abodes_;


Earth::Earth() noexcept = default;


Earth::Earth(Temporality& temporality)
{
	std::lock_guard lock(mutex_);
	if (temporality_ != nullptr)
		throw std::logic_error("Only one World");
	temporality_ = &temporality;

	bool saw_world_abode = false;
	for (Abode& place : temporality.abodes()) {
		const id::Abode id = place.id();
		if (id == id::Abode::global()) {
			saw_world_abode = true;
			continue; // Creation indexes the living World
		}
		auto owned = std::make_unique<Abode>(std::move(place));
		Abode* raw = owned.get();
		owned_abodes_.emplace(id, std::move(owned));
		abodes_.emplace(id, raw);
	}

	if (!saw_world_abode)
		throw std::logic_error("Earth requires the world abode in Temporality");
}


void Earth::roll() noexcept
{
	std::lock_guard lock(mutex_);
	vessels_.clear();
	id_by_token_.clear();
	abodes_.clear();
	owned_abodes_.clear();
	temporality_ = nullptr;
}


Temporality& Earth::temporality()
{
	if (temporality_ == nullptr)
		throw std::logic_error("Earth has not been brought forth");
	return *temporality_;
}


const Temporality& Earth::temporality() const
{
	if (temporality_ == nullptr)
		throw std::logic_error("Earth has not been brought forth");
	return *temporality_;
}


bool Earth::knows(const id::Vessel id) const
{
	std::lock_guard lock(mutex_);
	return vessels_.contains(id);
}


bool Earth::knows(const id::Abode id) const
{
	std::lock_guard lock(mutex_);
	return abodes_.contains(id);
}


Abode& Earth::abode(const id::Abode id)
{
	std::lock_guard lock(mutex_);
	const auto it = abodes_.find(id);
	if (it == abodes_.end() || !it->second)
		throw std::logic_error("Earth does not know this abode");
	return *it->second;
}


const Abode& Earth::abode(const id::Abode id) const
{
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


void Earth::fix(const id::Abode abode, const id::Soul author, const Word& word) const
{
	temporality().fix(abode, author, word);
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


void Earth::index(Abode& place)
{
	std::lock_guard lock(mutex_);
	abodes_.insert_or_assign(place.id(), &place);
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


} // namespace will::domain
