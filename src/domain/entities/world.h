#pragma once

#include "entities/abode.h"
#include "entities/earth.h"
#include "entities/heaven.h"
#include "entities/man.h"
#include "identity/vessel.h"
#include "ports/eternity.h"
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
	explicit World(Eternity& eternity);

	/// Single abode for now; later a registry of abodes in this world.
	Abode abode() const noexcept { return Abode::global(); }

	/// Man dwelling in this vessel. Throws if the vessel has no man (broken invariant).
	Man find_man_by_vessel(const Vessel& vessel) const;

	/// Birth a new man in Eternity and accept him into the living cosmos.
	Man birth_man(const DeviceToken& token);

private:
	void accept(Man man);

	mutable std::mutex mutex_;
	std::unordered_map<id::Man, std::unique_ptr<Man>> men_by_id_;
	std::unordered_map<id::Vessel, id::Man> man_id_by_vessel_;
};


} // namespace will::domain
