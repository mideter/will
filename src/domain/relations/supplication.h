#pragma once

#include "matter/supplication.h"
#include "properties/birth.h"
#include "properties/immanent.h"

#include <memory>


namespace will::domain {


class Heaven;
class Man;
class Novice;
class World;
class Shepherding;
class Testator;


/// Supplication (Просьба) — one asks another to become his testator; accepted, it binds
/// them in a tie.
class Supplication : public Immanent<Heaven> {
public:
	Supplication(Birth<Heaven>, matter::Supplication kept);

	const Man& suppliant() const noexcept { return suppliant_; }
	const Man& addressee() const noexcept { return addressee_; }

	const Shepherding& sign(const Testator& addressee) const;
	void reject(const Testator& addressee) const;

private:
	const Man& suppliant_;
	const Man& addressee_;
};


} // namespace will::domain
