#pragma once

#include "entities/abode.h"
#include "entities/earth.h"
#include "entities/heaven.h"
#include "entities/man.h"
#include "identity/vessel.h"
#include "ports/eternity.h"
#include "ports/letter_repository.h"
#include "values/device_token.h"

#include <memory>
#include <mutex>
#include <unordered_map>


namespace will::domain {


/// World (Мир) — living cosmos: Heaven, Earth, men, and abodes.
/// One object is both Heaven and Earth; speaks with Eternity through Heaven.
/// Men are heap-stable (unique_ptr); Heaven/Earth index the Soul/Vessel bases.
class World : public Heaven, public Earth {
public:
	World(Eternity& eternity, LetterRepository& letters);

	/// Single abode for now; later a registry of abodes in this world.
	Abode& abode() noexcept { return abode_; }
	const Abode& abode() const noexcept { return abode_; }

	/// Man dwelling in this vessel. Throws if the vessel has no man (broken invariant).
	Man find_man_by_vessel(const Vessel& vessel) const;

	/// Welcome a vessel's token: return the dwelling man, or beget one if unknown.
	Man welcome(const DeviceToken& token);

private:
	/// Beget a new man in Eternity and accept him into the living cosmos.
	Man beget(const DeviceToken& token);

	void accept(Man man);

	Abode abode_;
	mutable std::mutex mutex_;
	std::unordered_map<id::Man, std::unique_ptr<Man>> men_by_id_;
	std::unordered_map<id::Vessel, id::Man> man_id_by_vessel_;
};


} // namespace will::domain
