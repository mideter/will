#pragma once

#include "acts/embodiment.h"
#include "beings/soul.h"
#include "beings/vessel.h"
#include "identity/man.h"


namespace will::domain {


/// Man (Человек) — soul dwelling in a vessel. One object is both Soul and Vessel.
/// Only heirs construct Man; World births the living heap object as Testator.
/// Construction presents the living soul and vessel to Heaven and Earth.
class Man : public Soul, public Vessel {
public:
	~Man() override = default;

	id::Man id() const noexcept { return id_; }

	bool operator==(const Man& other) const = default;

	Man(const Man&) = delete;
	Man& operator=(const Man&) = delete;
	Man(Man&&) = delete;
	Man& operator=(Man&&) = delete;

protected:
	explicit Man(Embodiment embodiment);

private:
	id::Man id_;
};


} // namespace will::domain
