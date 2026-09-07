#pragma once

#include "entities/vessel.h"
#include "identity/vessel.h"
#include "values/device_token.h"

#include <mutex>
#include <unordered_map>


namespace will::domain {


/// Earth (Земля) — runtime index of vessels living in men.
/// Living access is through World (Мир), which is Earth.
/// Pointers address Vessel bases of heap-stable Man (unique_ptr).
/// Lookups hand out those living vessels — never snapshots.
class Earth {
public:
	/// Whether Earth knows a vessel by this token.
	bool knows(const DeviceToken& token) const;

	/// Living vessel for this token. Throws if unknown.
	const Vessel& vessel(const DeviceToken& token) const;

protected:
	Earth() = default;

	/// Index a vessel owned by a heap-stable Man.
	void index(const Vessel& vessel);

private:
	mutable std::mutex mutex_;
	std::unordered_map<id::Vessel, const Vessel*> vessels_by_id_;
	std::unordered_map<DeviceToken, id::Vessel> id_by_token_;
};


} // namespace will::domain
