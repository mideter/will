#pragma once

#include "matter/man.h"
#include "immanents/abode.h"
#include "immanents/man.h"
#include "values/saying.h"


namespace will::domain {


/// Witness (Свидетель) — living man; says in the waking world.
/// Awake, he contemplates exactly one thing — his own Abode unless he turns
/// elsewhere; asleep, nothing.
/// Further modes: Novice, Testator. Heap object is always Testator;
/// Witness is the base living mode.
class Witness : public Man {
public:
	/// Come to the waking world: contemplate one's own Abode.
	void wake() const;

	/// Leave the waking world: contemplate nothing.
	void sleep() const;

	/// Turn the gaze to an abode he dwells in. Throws if he does not dwell there.
	void contemplate(const Abode& abode) const;

	void say(const Saying& saying) const final;

protected:
	explicit Witness(matter::Man kept);
};


} // namespace will::domain
