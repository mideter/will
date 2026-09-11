#include "earth.h"

#include <algorithm>
#include <stdexcept>


namespace will::domain {


Temporality* Earth::temporality_ = nullptr;
std::mutex Earth::mutex_;
std::unordered_map<id::Vessel, const Vessel*> Earth::vessels_by_id_;
std::unordered_map<DeviceToken, id::Vessel> Earth::id_by_token_;


Earth::Earth(Temporality& temporality)
{
	std::lock_guard lock(mutex_);

	if (temporality_ != nullptr)
		throw std::logic_error("Only one World");

	temporality_ = &temporality;
}


Earth::~Earth()
{
	std::lock_guard lock(mutex_);
	vessels_by_id_.clear();
	id_by_token_.clear();
	temporality_ = nullptr;
}


bool Earth::knows(const id::Vessel id) const
{
	std::lock_guard lock(mutex_);
	return vessels_by_id_.contains(id);
}


const Vessel& Earth::vessel(const id::Vessel id) const
{
	std::lock_guard lock(mutex_);

	const auto it = vessels_by_id_.find(id);
	if (it == vessels_by_id_.end() || !it->second)
		throw std::logic_error("Earth does not know this vessel");

	return *it->second;
}


void Earth::fix(const id::Abode abode, const id::Soul author, const Word& word)
{
	temporality_->fix(abode, author, word);
}


std::vector<Letter> Earth::letters(const id::Abode abode, const std::uint32_t limit) const
{
	const std::uint32_t capped = std::min(limit, Temporality::MaxLetterLimit);
	return temporality_->letters(abode, capped);
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
	vessels_by_id_.insert_or_assign(vessel.id(), &vessel);
}


} // namespace will::domain
