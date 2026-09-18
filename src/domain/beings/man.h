#pragma once

#include "acts/embodiment.h"
#include "beings/soul.h"
#include "beings/vessel.h"


namespace will::domain {


/// Man (Человек) — soul dwelling in a vessel. One object is both Soul and Vessel.
/// Only heirs construct Man; World births the living heap object as Testator.
/// Construction presents the living soul and vessel to Heaven and Earth.
/// One soul — one living man in the World; living identity is Soul::id().
class Man : public Soul, public Vessel {
public:
	~Man() override = default;

	bool operator==(const Man& other) const = default;

protected:
	explicit Man(Embodiment embodiment);
};


} // namespace will::domain
