#pragma once

#include "identity/soul.h"


namespace will::domain::matter {


/// Awaits until its answer is kept.
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
