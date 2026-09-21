#pragma once


namespace will::domain {


class Novice;
class Testator;


/// Supplication (Прошение) — request to enter the shared Obedience/Shepherding place.
/// Pair (suppliant, addressee): future novice and the Testator asked to become Завещатель.
/// Pending while on the addressee's heap; each side signs (sign / reject).
class Supplication {
public:
	Supplication(const Novice& suppliant, const Testator& addressee);

	const Novice& suppliant() const noexcept { return suppliant_; }
	const Testator& addressee() const noexcept { return addressee_; }

	void sign(const Novice& novice) const;
	void sign(const Testator& testator) const;
	void reject(const Testator& testator) const;

private:
	const Novice& suppliant_;
	const Testator& addressee_;
};


} // namespace will::domain
