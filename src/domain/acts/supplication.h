#pragma once


namespace will::domain {


class Novice;
class Testator;


/// Supplication (Прошение) — request to enter the shared Obedience/Shepherding place.
/// Suppliant is the future novice; testator is asked to become Завещатель.
/// Holds living beings known to Heaven (stable while World lives).
/// Living identity is the ordered pair (suppliant, testator) while pending on the
/// testator's heap; presence is pending — closed rows stay only in Temporality.
///
/// Each side signs this act to authorize what is necessary on its behalf:
/// the novice at send (lodge; later keep Obedience), the testator at accept
/// (Shepherding, Temporality, drop) or reject. Roles do not reach each other.
class Supplication {
public:
	Supplication(const Novice& suppliant, const Testator& testator);

	const Novice& suppliant() const noexcept { return suppliant_; }
	const Testator& testator() const noexcept { return testator_; }

	/// Novice signs: lodge with the testator; on testator sign, keep Obedience.
	void sign(const Novice& novice) const;

	/// Testator signs: birth both faces in Temporality and on the heap.
	void sign(const Testator& testator) const;

	/// Testator rejects: refuse in Temporality and drop from the heap.
	void reject(const Testator& testator) const;

private:
	const Novice& suppliant_;
	const Testator& testator_;
};


} // namespace will::domain
