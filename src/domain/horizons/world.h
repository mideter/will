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
#include "values/timestamp.h"

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <unordered_map>


namespace will::domain {


/// World (Мир) — the one living cosmos, Heaven and Earth; it alone gives men birth.
class World : public Heaven, public Earth {
public:
	using Heaven::knows;
	using Earth::knows;

	~World();

	const Man& man(const Vessel& vessel) const;

	/// The man in this token's body, or an unborn body awaiting its soul; the first
	/// on an empty world is begotten at once.
	const Vessel& welcome(const DeviceToken& token);

	/// The same time by which words and efforts are dated.
	Timestamp now();

	/// In the order they came.
	std::vector<std::reference_wrapper<const Unborn>> unborn() const;

	const Unborn& unborn(id::Vessel id) const;

	/// Born of the host of the gates — his father by flesh — whoever bears him: the
	/// father becomes a neighbour in the child's abode, the child an acquaintance in his.
	const Man& bear(const Man& midwife, const Unborn& unborn);

	/// The father cannot refuse; the former one is replaced.
	void choose_father(const Man& child, const Man& father);

	struct Descent {
		std::reference_wrapper<const Man> child;
		std::reference_wrapper<const Man> father;
	};

	/// His children by spirit, theirs, and so on.
	std::vector<Descent> lineage(const Man& father) const;

	const Man& man(const SoulName& name) const;

	std::vector<std::reference_wrapper<const Man>> hosts_of(const Man& dweller) const;

	/// What was kept but does not agree among the dimensions — passed over on
	/// awakening, never erased — and what was made whole again.
	const std::vector<std::string>& unrecollected() const noexcept { return unrecollected_; }

private:
	friend class Creation;

	World(Eternity& eternity, Temporality& temporality, Spatiality& spatiality);

	void awaken();
	const Man& beget(const DeviceToken& token);
	const Unborn& await(matter::Vessel kept);
	const Man& accept(matter::Man kept);

	const Man& living_man(id::Soul id) const;
	bool lives(id::Soul id) const;

	mutable std::mutex mutex_;
	std::unordered_map<id::Soul, std::unique_ptr<Man>> men_;
	std::unordered_map<id::Vessel, id::Soul> soul_id_by_vessel_;
	std::vector<std::unique_ptr<Unborn>> unborn_;
	std::vector<std::string> unrecollected_;
};


} // namespace will::domain
