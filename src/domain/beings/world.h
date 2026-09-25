#pragma once

#include "acts/embodiment.h"
#include "beings/earth.h"
#include "beings/heaven.h"
#include "beings/man.h"
#include "identity/soul.h"
#include "identity/vessel.h"
#include "values/device_token.h"
#include "values/soul_name.h"

#include <memory>
#include <mutex>
#include <unordered_map>


namespace will::domain {


/// World (Мир) — the one living cosmos: is Heaven and Earth.
/// Heaven and Earth are in space and time; Time and Space are of Eternity.
/// Eternity is reached from Heaven. Each living man has a personal abode owned by his Witness.
/// Brought forth only by Creation as Heaven and Earth; Space already of Eternity.
/// Living people are Testator on the heap; World alone births them.
/// One soul — one living man; indexed by id::Soul.
class World : public Heaven, public Earth {
public:
	using Heaven::knows;
	using Earth::knows;

	~World() = default;

	/// Man dwelling in this vessel. Throws if the vessel has no man (broken invariant).
	const Man& man(const Vessel& vessel) const;

	/// Welcome a vessel's token: return the dwelling man, or beget one if unknown.
	const Man& welcome(const DeviceToken& token);

private:
	friend class Creation;

	World(Eternity& eternity, Temporality& temporality, Spatiality& spatiality);

	/// Accept remembered embodiments (Creation).
	void awaken();

	/// Enroll a soul, embody it in a vessel, birth Testator.
	const Man& beget(const DeviceToken& token);

	/// Place a Testator on the heap from embodiment material.
	const Man& accept(Embodiment embodiment);

	const Man& living_man(id::Soul id) const;

	mutable std::mutex mutex_;
	std::unordered_map<id::Soul, std::unique_ptr<Man>> men_;
	std::unordered_map<id::Vessel, id::Soul> soul_id_by_vessel_;
};


} // namespace will::domain
