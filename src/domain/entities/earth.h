#pragma once

#include "entities/heaven.h"
#include "entities/vessel.h"
#include "identity/vessel.h"
#include "values/device_token.h"

#include <mutex>
#include <optional>
#include <unordered_map>


namespace will::domain {


/// Earth (Земля) — runtime registry of vessels. Looks to Heaven; does not know Eternity.
class Earth {
public:
	explicit Earth(Heaven& heaven);

	std::optional<Vessel> find_vessel_by_token(const DeviceToken& token) const;

	void insert(Vessel vessel);

private:
	Heaven& heaven_;
	mutable std::mutex mutex_;
	std::unordered_map<id::Vessel, Vessel> vessels_by_id_;
	std::unordered_map<DeviceToken, id::Vessel> id_by_token_;
};


} // namespace will::domain
