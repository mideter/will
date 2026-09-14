#pragma once

#include "entities/abode.h"
#include "entities/earth.h"
#include "entities/heaven.h"
#include "entities/letter.h"
#include "entities/man.h"
#include "identity/man.h"
#include "identity/vessel.h"
#include "ports/temporality.h"
#include "values/device_token.h"

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>


namespace will::domain {


/// World (Мир) — the one living cosmos: is Heaven and Earth, holds living men.
/// Abodes live on Earth; souls on Heaven. Raises and clears the static poles in
/// ctor/dtor. Living people are Witness on the heap (unique_ptr<Man>); APIs hand
/// out Man&. Eternity still deals in Man values.
class World : public Heaven, public Earth {
public:
	using Earth::abode;
	using Earth::knows;
	using Heaven::knows;

	explicit World(Temporality& temporality);
	~World() = default;

	/// Man dwelling in this vessel. Throws if the vessel has no man (broken invariant).
	const Man& man(const Vessel& vessel) const;

	/// Welcome a vessel's token: return the dwelling man, or beget one if unknown.
	const Man& welcome(const DeviceToken& token);

	/// History of the global abode through Temporality (capped).
	std::vector<Letter> letters(std::uint32_t limit) const;

private:
	/// Beget a new man in Eternity and accept him into the living cosmos as Witness.
	const Man& beget(const DeviceToken& token);

	/// Place a man on the heap as Witness observing the global abode.
	const Man& accept(Man&& man);

	const Man& living_man(id::Man id) const;
	void restore_dwellers();

	mutable std::mutex mutex_;
	std::unordered_map<id::Man, std::unique_ptr<Man>> men_;
	std::unordered_map<id::Vessel, id::Man> man_id_by_vessel_;
};


} // namespace will::domain
