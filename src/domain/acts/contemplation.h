#pragma once

#include "beings/immanents/abode.h"


namespace will::domain {


/// Contemplation (Созерцание) — relation in Heaven: which Abode a soul observes.
class Contemplation {
public:
	explicit Contemplation(const Abode& abode);

	const Abode& abode() const noexcept { return *abode_; }

private:
	const Abode* abode_;
};


} // namespace will::domain
