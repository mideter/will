#pragma once

#include "identity/supplication.h"
#include "values/timestamp.h"

#include <cstdint>


namespace will::domain {


class Executor;
class Obedience;
class Soul;
class Testator;


enum class SupplicationStatus : std::uint8_t {
	pending,
	accepted,
	refused,
	withdrawn,
};


/// Supplication (Прошение) — request to enter the shared Obedience/Shepherding place.
/// Suppliant is the future executor; testator is asked to become Завещатель.
/// Holds living souls known to Heaven (stable while World lives).
///
/// Each side entrusts this act to do what is necessary on its behalf:
/// the executor at send (lodge; later keep Obedience), the testator at consent
/// (Shepherding, Temporality, drop). Roles do not reach each other.
class Supplication {
public:
	Supplication(id::Supplication id, const Soul& suppliant, const Soul& testator,
				 SupplicationStatus status, Timestamp created_at);

	id::Supplication id() const noexcept { return id_; }
	const Soul& suppliant() const noexcept { return suppliant_; }
	const Soul& testator() const noexcept { return testator_; }
	SupplicationStatus status() const noexcept { return status_; }
	Timestamp created_at() const noexcept { return created_at_; }

	/// Executor entrusts this pending act: lodge with the testator; on consent, keep Obedience.
	void entrust(const Executor& executor) const;

	/// Testator entrusts this pending act: birth both faces in Temporality and on the heap.
	const Obedience& consent(const Testator& testator) const;

	/// Testator entrusts this pending act: refuse in Temporality and drop from the heap.
	void dismiss(const Testator& testator) const;

private:
	id::Supplication id_;
	const Soul& suppliant_;
	const Soul& testator_;
	SupplicationStatus status_;
	Timestamp created_at_;
};


} // namespace will::domain
