#pragma once

#include "identity/soul.h"


namespace will::domain::matter {


/// Matter of living Supplication (Прошение) — suppliant asks addressee to become
/// Завещатель, fixed in time. Temporality keeps it; it awaits answer until the
/// pair is bound (matter::Tie in Spatiality) or its rejection is kept.
class Supplication {
public:
	Supplication(id::Soul suppliant, id::Soul addressee);

	id::Soul suppliant() const noexcept { return suppliant_; }
	id::Soul addressee() const noexcept { return addressee_; }

private:
	id::Soul suppliant_;
	id::Soul addressee_;
};


} // namespace will::domain::matter
