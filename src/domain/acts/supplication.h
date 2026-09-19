#pragma once

#include "identity/supplication.h"
#include "values/timestamp.h"

#include <cstdint>


namespace will::domain {


class Soul;


enum class SupplicationStatus : std::uint8_t {
	pending,
	accepted,
	refused,
	withdrawn,
};


/// Supplication (Прошение) — request to enter the shared Obedience/Shepherding place.
/// Suppliant is the future executor; addressee is the future testator.
/// Holds living souls known to Heaven (stable while World lives).
class Supplication {
public:
	Supplication(id::Supplication id, const Soul& suppliant, const Soul& addressee,
				 SupplicationStatus status, Timestamp created_at);

	id::Supplication id() const noexcept { return id_; }
	const Soul& suppliant() const noexcept { return suppliant_; }
	const Soul& addressee() const noexcept { return addressee_; }
	SupplicationStatus status() const noexcept { return status_; }
	Timestamp created_at() const noexcept { return created_at_; }

private:
	id::Supplication id_;
	const Soul& suppliant_;
	const Soul& addressee_;
	SupplicationStatus status_;
	Timestamp created_at_;
};


} // namespace will::domain
