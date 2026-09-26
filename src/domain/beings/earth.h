#pragma once

#include "properties/immanent.h"
#include "identity/vessel.h"
#include "values/device_token.h"

#include <mutex>
#include <optional>
#include <unordered_map>


namespace will::domain {


class Vessel;
class Temporality;
class Spatiality;


/// Earth (Земля) — symbol of the visible world.
/// Speaks with Temporality and Spatiality.
/// Dust reaches Temporality and Spatiality via friendship.
/// Earth is in space and time. Live registry of vessels; abodes live with men;
/// places are known to Space; spatial material to Spatiality.
class Earth : private Immanent<Earth> {
public:
	bool knows(id::Vessel id) const;

	/// Throws if unknown.
	const Vessel& vessel(id::Vessel id) const;

protected:
	friend class Dust;
	friend class Immanent<Earth>;

	/// As World.
	Earth(Temporality& temporality, Spatiality& spatiality);

	~Earth();

	/// For World::welcome. Empty if unknown.
	std::optional<id::Vessel> id_of(const DeviceToken& token) const;

	Temporality& temporality() noexcept { return temporality_; }
	const Temporality& temporality() const noexcept { return temporality_; }

	Spatiality& spatiality() noexcept { return spatiality_; }
	const Spatiality& spatiality() const noexcept { return spatiality_; }

private:
	/// Vessel owned by a heap-stable Man.
	void present(const Vessel& vessel);

	/// Throws if not yet brought forth / already destroyed.
	static Earth& the();

	static Earth* current_;

	Temporality& temporality_;
	Spatiality& spatiality_;
	mutable std::mutex mutex_;
	std::unordered_map<id::Vessel, const Vessel*> vessels_;
	std::unordered_map<DeviceToken, id::Vessel> id_by_token_;
};


} // namespace will::domain
