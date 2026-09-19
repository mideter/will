#pragma once

#include "values/timestamp.h"

#include <cstdint>


namespace will::domain {


class Executor;
class Testator;


enum class SupplicationStatus : std::uint8_t {
	pending,
	accepted,
	refused,
	withdrawn,
};


/// Supplication (Прошение) — request to enter the shared Obedience/Shepherding place.
/// Suppliant is the future executor; testator is asked to become Завещатель.
/// Holds living beings known to Heaven (stable while World lives).
/// Living identity is the ordered pair (suppliant, testator) while pending.
///
/// Each side signs this act to authorize what is necessary on its behalf:
/// the executor at send (lodge; later keep Obedience), the testator at accept
/// (Shepherding, Temporality, drop) or reject. Roles do not reach each other.
class Supplication {
public:
	Supplication(const Executor& suppliant, const Testator& testator, SupplicationStatus status,
				 Timestamp created_at);

	const Executor& suppliant() const noexcept { return suppliant_; }
	const Testator& testator() const noexcept { return testator_; }
	SupplicationStatus status() const noexcept { return status_; }
	Timestamp created_at() const noexcept { return created_at_; }

	/// Executor signs: lodge with the testator; on testator sign, keep Obedience.
	void sign(const Executor& executor) const;

	/// Testator signs: birth both faces in Temporality and on the heap.
	void sign(const Testator& testator) const;

	/// Testator rejects: refuse in Temporality and drop from the heap.
	void reject(const Testator& testator) const;

private:
	const Executor& suppliant_;
	const Testator& testator_;
	SupplicationStatus status_;
	Timestamp created_at_;
};


} // namespace will::domain
