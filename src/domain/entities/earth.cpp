#include "earth.h"

#include <stdexcept>


namespace will::domain {


bool Earth::knows(const DeviceToken& token) const
{
	std::lock_guard lock(mutex_);
	return id_by_token_.contains(token);
}


bool Earth::knows(const id::Vessel id) const
{
	std::lock_guard lock(mutex_);
	return vessels_by_id_.contains(id);
}


const Vessel& Earth::vessel(const DeviceToken& token) const
{
	std::lock_guard lock(mutex_);

	const auto token_it = id_by_token_.find(token);
	if (token_it == id_by_token_.end())
		throw std::logic_error("Earth does not know this vessel token");

	const auto it = vessels_by_id_.find(token_it->second);
	if (it == vessels_by_id_.end() || !it->second)
		throw std::logic_error("Earth does not know this vessel token");

	return *it->second;
}


const Vessel& Earth::vessel(const id::Vessel id) const
{
	std::lock_guard lock(mutex_);

	const auto it = vessels_by_id_.find(id);
	if (it == vessels_by_id_.end() || !it->second)
		throw std::logic_error("Earth does not know this vessel");

	return *it->second;
}


void Earth::index(const Vessel& vessel)
{
	std::lock_guard lock(mutex_);
	id_by_token_.insert_or_assign(vessel.token(), vessel.id());
	vessels_by_id_.insert_or_assign(vessel.id(), &vessel);
}


} // namespace will::domain
