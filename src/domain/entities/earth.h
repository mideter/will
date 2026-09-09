#pragma once

#include "entities/vessel.h"
#include "identity/vessel.h"
#include "values/device_token.h"

#include <mutex>
#include <optional>
#include <unordered_map>


namespace will::domain {


/// Earth (Земля) — runtime index of vessels living in men.
/// Living access is through World (Мир), which is Earth.
/// Public face is by vessel id (like Heaven by soul id).
/// Pointers address Vessel bases of heap-stable Man (unique_ptr).
/// Lookups hand out those living vessels — never snapshots.
class Earth {
public:
	/// Whether Earth knows this vessel.
	bool knows(id::Vessel id) const;

	/// Living vessel by id. Throws if unknown.
	const Vessel& vessel(id::Vessel id) const;

protected:
	Earth() = default;

	/// Index a vessel owned by a heap-stable Man.
	void index(const Vessel& vessel);

	/// Resolve device token to vessel id for World::welcome. Empty if unknown.
	std::optional<id::Vessel> id_of(const DeviceToken& token) const;

private:
	mutable std::mutex mutex_;
	std::unordered_map<id::Vessel, const Vessel*> vessels_by_id_;
	std::unordered_map<DeviceToken, id::Vessel> id_by_token_;
};


} // namespace will::domain
