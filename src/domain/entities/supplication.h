#pragma once

#include "identity/soul.h"
#include "identity/supplication.h"
#include "values/timestamp.h"

#include <cstdint>


namespace will::domain {


enum class SupplicationStatus : std::uint8_t {
	pending,
	accepted,
	refused,
	withdrawn,
};


/// Supplication (Прошение) — request to enter Obedience.
/// Suppliant is the future executor; addressee is the future testator.
/// Living souls are reached through Spirit/Heaven by these ids.
class Supplication {
public:
	Supplication(id::Supplication id, id::Soul suppliant, id::Soul addressee, SupplicationStatus status,
				 Timestamp created_at);

	id::Supplication id() const noexcept { return id_; }
	id::Soul suppliant_id() const noexcept { return suppliant_id_; }
	id::Soul addressee_id() const noexcept { return addressee_id_; }
	SupplicationStatus status() const noexcept { return status_; }
	Timestamp created_at() const noexcept { return created_at_; }

	bool operator==(const Supplication&) const = default;

private:
	id::Supplication id_;
	id::Soul suppliant_id_;
	id::Soul addressee_id_;
	SupplicationStatus status_;
	Timestamp created_at_;
};


} // namespace will::domain
