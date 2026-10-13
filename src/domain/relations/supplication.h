#pragma once

#include "matter/supplication.h"
#include "properties/birth.h"
#include "properties/immanent.h"

#include <memory>


namespace will::domain {


class Heaven;
class Novice;
class World;
class Shepherding;
class Testator;


/// Supplication (Просьба) — one asks another to become his testator; accepted, it binds
/// them in a tie.
class Supplication : public Immanent<Heaven> {
public:
	Supplication(Birth<Novice>, matter::Supplication kept);
	Supplication(Birth<World>, matter::Supplication kept);

	const Novice& suppliant() const noexcept { return suppliant_; }
	const Testator& addressee() const noexcept { return addressee_; }

	const Shepherding& sign(const Testator& addressee) const;
	void reject(const Testator& addressee) const;

private:
	explicit Supplication(matter::Supplication kept);

	const Novice& suppliant_;
	const Testator& addressee_;
};


} // namespace will::domain
