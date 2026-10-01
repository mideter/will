#pragma once

#include "acts/asking.h"


namespace will::domain {


class Novice;
class Shepherding;
class Testator;


/// Supplication (Прошение) — request to enter the shared Obedience/Shepherding place.
/// Pair (suppliant, addressee): future novice and the Testator asked to become Завещатель.
/// Born from Asking (Испрашивание). Pending while on the addressee's heap; each
/// side signs (sign / reject). Signed by the addressee, it performs the Tying
/// (Связывание) on his behalf and the living Tie is born.
class Supplication {
public:
	/// Both souls must be known to Heaven.
	explicit Supplication(Asking asking);

	const Novice& suppliant() const noexcept { return suppliant_; }
	const Testator& addressee() const noexcept { return addressee_; }

	void sign(const Novice& suppliant) const;
	const Shepherding& sign(const Testator& addressee) const;
	void reject(const Testator& addressee) const;

private:
	const Novice& suppliant_;
	const Testator& addressee_;
};


} // namespace will::domain
