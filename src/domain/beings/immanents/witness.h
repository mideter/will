#pragma once

#include "matter/man.h"
#include "beings/immanents/abode.h"
#include "beings/immanents/man.h"
#include "values/saying.h"


namespace will::domain {


/// Witness (Свидетель) — living man; says in the waking world.
/// Exercises Contemplation (initially his own Abode).
/// Further modes: Novice, Testator. Heap object is always Testator;
/// Witness is the base living mode.
class Witness : public Man {
public:
	void contemplate(const Abode& abode) const;

	void say(const Saying& saying) const final;

protected:
	explicit Witness(matter::Man kept);
};


} // namespace will::domain
