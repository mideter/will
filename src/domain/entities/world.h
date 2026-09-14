#pragma once

#include "entities/abode.h"
#include "entities/earth.h"
#include "entities/heaven.h"
#include "entities/letter.h"
#include "entities/man.h"
#include "identity/vessel.h"
#include "ports/temporality.h"
#include "values/device_token.h"

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>


namespace will::domain {


/// World (Мир) — the one living cosmos: is Heaven and Earth, holds men and abodes.
/// Raises and clears the static poles in ctor/dtor. Living people are Witness on the
/// heap (unique_ptr<Man>); APIs hand out Man&. Eternity still deals in Man values.
class World : public Heaven, public Earth {
public:
	using Earth::knows;
	using Heaven::knows;

	explicit World(Temporality& temporality);
	~World() = default;

	/// Whether the world holds this abode.
	bool knows(id::Abode id) const;

	/// Living abode by id. Throws if unknown.
	Abode& abode(id::Abode id);
	const Abode& abode(id::Abode id) const;

	/// Global abode (id 1) — default place until birth/choice of others.
	Abode& abode() { return abode(id::Abode::global()); }
	const Abode& abode() const { return abode(id::Abode::global()); }

	/// Man dwelling in this vessel. Throws if the vessel has no man (broken invariant).
	const Man& man(const Vessel& vessel) const;

	/// Welcome a vessel's token: return the dwelling man, or beget one if unknown.
	const Man& welcome(const DeviceToken& token);

	/// History of the global abode through Earth / Temporality (capped).
	std::vector<Letter> letters(std::uint32_t limit) const;

private:
	/// Beget a new man in Eternity and accept him into the living cosmos as Witness.
	const Man& beget(const DeviceToken& token);

	/// Place a man on the heap as Witness observing the global abode.
	const Man& accept(Man&& man);

	mutable std::mutex mutex_;
	std::unordered_map<id::Abode, std::unique_ptr<Abode>> abodes_by_id_;
	std::unordered_map<id::Man, std::unique_ptr<Man>> men_by_id_;
	std::unordered_map<id::Vessel, id::Man> man_id_by_vessel_;
};


} // namespace will::domain
