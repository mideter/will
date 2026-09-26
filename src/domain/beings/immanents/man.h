#pragma once

#include "acts/embodiment.h"
#include "beings/immanents/abode.h"
#include "beings/immanents/soul.h"
#include "beings/immanents/vessel.h"

#include <memory>


namespace will::domain {


/// Man (Человек) — soul dwelling in a vessel. One object is both Soul and Vessel.
/// Owns his personal Abode. Only heirs construct Man; World births the living
/// heap object as Testator. Construction presents soul and vessel to Heaven and Earth.
/// One soul — one living man in the World; living identity is Soul::id().
class Man : public Soul, public Vessel {
public:
	~Man() override;

	Abode& abode() const noexcept { return *abode_; }

	bool operator==(const Man& other) const = default;

protected:
	explicit Man(Embodiment embodiment);

private:
	std::unique_ptr<Abode> abode_;
};


} // namespace will::domain
