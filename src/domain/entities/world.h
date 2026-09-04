#pragma once

#include "entities/abode.h"
#include "entities/earth.h"
#include "entities/heaven.h"
#include "entities/man.h"
#include "identity/soul.h"
#include "identity/vessel.h"
#include "ports/eternity.h"
#include "values/device_token.h"

#include <mutex>
#include <optional>
#include <unordered_map>


namespace will::domain {


/// World (Мир) — living cosmos: Heaven, Earth, men, and abodes.
/// Speaks with Eternity only through Heaven.
class World {
public:
	explicit World(Eternity& eternity);

	Heaven& heaven() noexcept { return heaven_; }
	const Heaven& heaven() const noexcept { return heaven_; }

	Earth& earth() noexcept { return earth_; }
	const Earth& earth() const noexcept { return earth_; }

	/// Single abode for now; later a registry of abodes in this world.
	Abode abode() const noexcept { return Abode::global(); }

	std::optional<Man> find_man_by_token(const DeviceToken& token) const;
	std::optional<id::Soul> soul_id_for_token(const DeviceToken& token) const;

	/// Birth a new man in Eternity (via Heaven) and accept soul, vessel, and man into the living cosmos.
	Man birth_man(const DeviceToken& token);

	void insert(Man man);

private:
	Heaven heaven_;
	Earth earth_;
	mutable std::mutex mutex_;
	std::unordered_map<id::Man, Man> men_by_id_;
	std::unordered_map<id::Vessel, id::Man> man_id_by_vessel_;
};


} // namespace will::domain
