#pragma once

#include "matter/man.h"
#include "places/abode.h"
#include "men/man.h"
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

	/// Turn the gaze to a place he dwells in — his abode, or a tie he is a side
	/// of. Throws if he does not dwell there.
	void contemplate(const Place& place) const;

	/// Whether this witness contemplates this place now.
	bool contemplates(const Place& place) const;

	void say(const Saying& saying) const final;

protected:
	explicit Witness(matter::Man kept);
};


} // namespace will::domain
