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


/// Supplication (Прошение) — request to enter the shared Obedience/Shepherding place.
/// Pair (suppliant, addressee): future novice and the Testator asked to become Завещатель.
/// Born from matter::Supplication. Living: a relation of one soul to another,
/// immanent to Heaven, neither copied nor moved. The suppliant submits it to the
/// addressee, who hears it and keeps it until he answers (sign / reject).
/// Signed by the addressee, it binds the pair on his behalf: matter::Tie is kept
/// and the living Tie is born.
class Supplication : public Immanent<Heaven> {
public:
	/// Both souls must be known to Heaven.
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
