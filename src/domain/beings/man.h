#pragma once

#include "acts/embodiment.h"
#include "beings/hypostasis.h"
#include "beings/soul.h"
#include "beings/vessel.h"


namespace will::domain {


/// Man (Человек) — soul dwelling in a vessel. One object is both Soul and Vessel.
/// Only heirs construct Man; World births the living heap object as Testator.
/// Construction presents the living soul and vessel to Heaven and Earth.
class Man : public Soul, public Vessel, private Hypostasis {
public:
	~Man() override = default;

	using Hypostasis::id;

	bool operator==(const Man& other) const = default;

protected:
	explicit Man(Embodiment embodiment);
};


} // namespace will::domain
