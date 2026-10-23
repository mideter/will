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


/// The one living cosmos, Heaven and Earth; it alone gives men birth. It welcomes bodies one
/// at a time, each by its token: the man in it, or an unborn body awaiting its soul, kept in
/// the order they came; the first on an empty world is begotten at once. A child is born at the gates of
/// his father by flesh, its host, who becomes a neighbour in the child's abode and the child
/// an acquaintance in his. A father by spirit cannot refuse, and replaces the former; his
/// lineage is his children by spirit, theirs, and so on. Its time is the one words and
/// efforts are dated by. What was kept but does not agree among the dimensions it passes
/// over on awakening, never erasing it, and tells it with what was made whole again.
class World : public Heaven, public Earth {
public:
	using Heaven::knows;
	using Earth::knows;

	~World();

	const Man& man(const Vessel& vessel) const;

	const Vessel& welcome(const DeviceToken& token);

	Timestamp now();

	std::vector<std::reference_wrapper<const Unborn>> unborn() const;

	const Unborn& unborn(id::Vessel id) const;

	void choose_father(const Man& child, const Man& father);

	struct Descent {
		std::reference_wrapper<const Man> child;
		std::reference_wrapper<const Man> father;
	};

	std::vector<Descent> lineage(const Man& father) const;

	const Man& man(const SoulName& name) const;

	std::vector<std::reference_wrapper<const Man>> hosts_of(const Man& dweller) const;

	const std::vector<std::string>& unrecollected() const noexcept { return unrecollected_; }

private:
	friend class Creation;

	World(Eternity& eternity, Temporality& temporality, Spatiality& spatiality);

	void awaken();
	const Man& beget(const DeviceToken& token);
	const Unborn& await(matter::Vessel kept);
	const Man& accept(matter::Man kept);

	void ask(const Spirit& suppliant, const Soul& addressee) override;

	const Man& bear(Birth<Gates> gates, const Unborn& unborn) override;

	const Man& living_man(id::Soul id) const;
	bool lives(id::Soul id) const;

	mutable std::mutex mutex_;
	std::mutex welcoming_;
	std::unordered_map<id::Soul, std::unique_ptr<Man>> men_;
	std::unordered_map<id::Vessel, id::Soul> soul_id_by_vessel_;
	std::vector<std::unique_ptr<Unborn>> unborn_;
	std::vector<std::string> unrecollected_;
};


} // namespace will::domain
