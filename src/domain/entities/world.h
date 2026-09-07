#pragma once

#include "entities/abode.h"
#include "entities/earth.h"
#include "entities/heaven.h"
#include "entities/man.h"
#include "identity/vessel.h"
#include "ports/temporality.h"
#include "values/device_token.h"

#include <memory>
#include <mutex>
#include <unordered_map>


namespace will::domain {


/// World (Мир) — living cosmos: Heaven, Earth, men, and abodes.
/// Men are heap-stable (unique_ptr). APIs that name a living man, soul, or vessel
/// hand out references/pointers into that storage — never snapshots.
class World : public Heaven, public Earth {
public:
	using Earth::knows;
	using Heaven::knows;

	explicit World(Temporality& temporality);

	/// Single abode for now; later a registry of abodes in this world.
	Abode& abode() noexcept { return abode_; }
	const Abode& abode() const noexcept { return abode_; }

	/// Man dwelling in this vessel. Throws if the vessel has no man (broken invariant).
	const Man& man(const Vessel& vessel) const;

	/// Welcome a vessel's token: return the dwelling man, or beget one if unknown.
	const Man& welcome(const DeviceToken& token);

private:
	/// Beget a new man in Eternity and accept him into the living cosmos.
	const Man& beget(const DeviceToken& token);

	/// Place a man on the heap and index him. Returns the living man.
	const Man& accept(Man&& man);

	Abode abode_;
	mutable std::mutex mutex_;
	std::unordered_map<id::Man, std::unique_ptr<Man>> men_by_id_;
	std::unordered_map<id::Vessel, id::Man> man_id_by_vessel_;
};


} // namespace will::domain
