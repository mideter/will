#pragma once

#include "beings/immanents/abode.h"


namespace will::domain {


/// Contemplation (Созерцание) — which Abode a Witness observes.
/// Owned by the Witness; the Abode does not keep its contemplators.
class Contemplation {
public:
	explicit Contemplation(const Abode& abode);

	const Abode& abode() const noexcept { return *abode_; }

private:
	const Abode* abode_;
};


} // namespace will::domain
