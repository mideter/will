#pragma once

#include "identity/soul.h"


namespace will::domain {


/// Asking (Испрашивание) — suppliant asks addressee to become Завещатель, fixed in time.
/// Matter for living Supplication. Temporality keeps Askings; one awaits answer
/// until the pair is bound (Boundness in Spatiality) or its rejection is kept.
class Asking {
public:
	Asking(id::Soul suppliant, id::Soul addressee);

	id::Soul suppliant() const noexcept { return suppliant_; }
	id::Soul addressee() const noexcept { return addressee_; }

private:
	id::Soul suppliant_;
	id::Soul addressee_;
};


} // namespace will::domain
