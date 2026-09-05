#include "earth.h"


namespace will::domain {


std::optional<Vessel> Earth::find_vessel_by_token(const DeviceToken& token) const
{
	std::lock_guard lock(mutex_);

	const auto token_it = id_by_token_.find(token);
	if (token_it == id_by_token_.end())
		return std::nullopt;

	const auto it = vessels_by_id_.find(token_it->second);
	if (it == vessels_by_id_.end())
		return std::nullopt;

	return *it->second;
}


void Earth::index(const Vessel& vessel)
{
	std::lock_guard lock(mutex_);
	id_by_token_.insert_or_assign(vessel.token(), vessel.id());
	vessels_by_id_.insert_or_assign(vessel.id(), &vessel);
}


} // namespace will::domain
