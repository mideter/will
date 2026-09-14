#pragma once

#include "entities/abode.h"
#include "entities/earth.h"
#include "entities/heaven.h"
#include "entities/man.h"
#include "identity/abode.h"
#include "identity/man.h"
#include "identity/soul.h"
#include "identity/vessel.h"
#include "values/device_token.h"

#include <memory>
#include <mutex>
#include <unordered_map>


namespace will::domain {


/// World (Мир) — the one living cosmos: is Heaven, Earth, and Abode.
/// The World itself is the global abode (id 1, name "world").
/// Brought forth only by Creation with Heaven and Earth already brought forth.
/// Living people are Witness on the heap (unique_ptr<Man>);
/// Eternity still deals in Man values. Heaven and Earth are rolled up by Creation.
class World : public Heaven, public Earth, public Abode {
public:
	using Heaven::knows;
	using Earth::knows;
	using Earth::abode;

	~World() = default;

	World(const World&) = delete;
	World& operator=(const World&) = delete;
	World(World&&) = delete;
	World& operator=(World&&) = delete;

	/// Man dwelling in this vessel. Throws if the vessel has no man (broken invariant).
	const Man& man(const Vessel& vessel) const;

	/// Welcome a vessel's token: return the dwelling man, or beget one if unknown.
	const Man& welcome(const DeviceToken& token);

private:
	friend class Creation;

	World(Heaven heaven, Earth earth);

	/// Index on Earth, then accept remembered men and restore dwellers (Creation).
	void awaken();

	/// Beget a new man in Eternity and accept him into the living cosmos as Witness.
	const Man& beget(const DeviceToken& token);

	/// Place a man on the heap as Witness observing the World-abode.
	const Man& accept(Man&& man);

	const Man& living_man(id::Man id) const;
	void restore_dwellers();

	mutable std::mutex mutex_;
	std::unordered_map<id::Man, std::unique_ptr<Man>> men_;
	std::unordered_map<id::Vessel, id::Man> man_id_by_vessel_;
};


} // namespace will::domain
