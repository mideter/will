#pragma once

#include "matter/man.h"
#include "beings/letter.h"
#include "beings/immanents/abode.h"
#include "beings/immanents/man.h"
#include "values/saying.h"

#include <cstdint>
#include <vector>


namespace will::domain {


/// Witness (Свидетель) — living man; say and retell in the waking world.
/// Exercises Contemplation (initially his own Abode).
/// Further modes: Novice, Testator. Heap object is always Testator;
/// Witness is the base living mode.
class Witness : public Man {
public:
	void contemplate(const Abode& abode) const;

	void say(const Saying& saying) const final;

	/// Letters of the contemplated abode (capped).
	std::vector<Letter> retell(std::uint32_t limit) const;

protected:
	explicit Witness(matter::Man kept);
};


} // namespace will::domain
