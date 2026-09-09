#include "earth.h"

#include <algorithm>
#include <stdexcept>


namespace will::domain {


Earth::Earth(Temporality& temporality)
	: temporality_(temporality)
{}


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
	temporality_.fix(abode, author, word);
}


std::vector<Letter> Earth::letters(const id::Abode abode, const std::uint32_t limit) const
{
	const std::uint32_t capped = std::min(limit, Temporality::MaxLetterLimit);
	return temporality_.letters(abode, capped);
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
