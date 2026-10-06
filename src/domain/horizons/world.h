#pragma once

#include "matter/man.h"
#include "horizons/earth.h"
#include "horizons/heaven.h"
#include "places/abode.h"
#include "men/man.h"
#include "men/unborn.h"
#include "identity/soul.h"
#include "identity/vessel.h"
#include "values/device_token.h"
#include "values/soul_name.h"

#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include <unordered_map>


namespace will::domain {


/// World (Мир) — the one living cosmos: Heaven and Earth as invisible and visible.
/// In space and time; Time and Space of Eternity.
/// Eternity is reached from Heaven. Each living man has a personal abode.
/// Brought forth only by Creation as Heaven and Earth; Space already of Eternity.
/// Living people are Testator on the heap; World alone births them.
/// One soul — one living man; indexed by id::Soul.
class World : public Heaven, public Earth {
public:
	using Heaven::knows;
	using Earth::knows;

	~World();

	/// Man dwelling in this vessel. Throws if the vessel has no man (broken invariant).
	const Man& man(const Vessel& vessel) const;

	/// Welcome a vessel's token: the man dwelling in its body, or the unborn
	/// body awaiting its soul. An unknown token is given a body to await its
	/// birth — unless no man is yet: the first is begotten at once.
	const Vessel& welcome(const DeviceToken& token);

	/// The bodies awaiting their soul, in the order they came.
	std::vector<std::reference_wrapper<const Unborn>> unborn() const;

	/// The unborn body with this id. Throws (std::invalid_argument) if none awaits.
	const Unborn& unborn(id::Vessel id) const;

	/// A midwife standing in a birth room bears the unborn: a soul is enrolled
	/// into its body, and the man is born of the host of that room, his father by
	/// flesh. The father becomes a neighbour in the child's abode, the child an
	/// acquaintance in the father's. Throws if the midwife stands elsewhere.
	const Man& bear(const Man& midwife, const Unborn& unborn);

	/// Living man by public soul name. Throws if unknown.
	const Man& man(const SoulName& name) const;

	/// The hosts in whose abodes this man dwells.
	std::vector<std::reference_wrapper<const Man>> hosts_of(const Man& dweller) const;

private:
	friend class Creation;

	World(Eternity& eternity, Temporality& temporality, Spatiality& spatiality);

	/// Accept kept men (Creation).
	void awaken();

	/// Enroll a soul, embody it in a vessel, birth Testator.
	const Man& beget(const DeviceToken& token);

	/// A body awaits its soul.
	const Unborn& await(matter::Vessel kept);

	/// Place a Testator on the heap from its matter.
	const Man& accept(matter::Man kept);

	const Man& living_man(id::Soul id) const;

	mutable std::mutex mutex_;
	std::unordered_map<id::Soul, std::unique_ptr<Man>> men_;
	std::unordered_map<id::Vessel, id::Soul> soul_id_by_vessel_;
	std::vector<std::unique_ptr<Unborn>> unborn_;
};


} // namespace will::domain
