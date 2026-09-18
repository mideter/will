#pragma once

#include "identity/soul.h"
#include "ports/time.h"
#include "values/soul_name.h"


namespace will::domain {


/// Eternity (Вечность) — who endures.
/// Time belongs to Eternity; without the eternal there is no present for the temporal.
/// Speaks in souls; embodiment in a vessel belongs to Temporality / World.
class Eternity {
public:
	virtual ~Eternity() = default;

	/// Time given by eternity.
	virtual Time& time() = 0;

	/// Enroll a soul in the book of life.
	virtual id::Soul enroll(SoulName name) = 0;
};


} // namespace will::domain
