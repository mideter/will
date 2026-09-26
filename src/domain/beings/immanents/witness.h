#pragma once

#include "acts/contemplation.h"
#include "acts/embodiment.h"
#include "beings/letter.h"
#include "beings/immanents/man.h"
#include "values/saying.h"

#include <cstdint>
#include <vector>


namespace will::domain {


/// Witness (Свидетель) — living man; say and retell in the waking world.
/// Contemplates one Abode (initially his own). Further modes: Novice, Testator.
/// Heap object is always Testator; Witness is the base living mode.
class Witness : public Man {
public:
	const Contemplation& contemplation() const noexcept { return contemplation_; }

	void say(const Saying& saying) const final;

	/// Outward witnessing: retell letters of the contemplated abode (capped).
	std::vector<Letter> retell(std::uint32_t limit) const;

protected:
	explicit Witness(Embodiment embodiment);

private:
	Contemplation contemplation_;
};


} // namespace will::domain
