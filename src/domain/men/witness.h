#pragma once

#include "matter/man.h"
#include "places/abode.h"
#include "men/man.h"
#include "values/saying.h"


namespace will::domain {


class Room;


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

	/// Turn the gaze to an abode he dwells in, or to a room of it he enters: a
	/// tie is seen through the room reflecting it. Throws if he does not dwell there.
	void contemplate(const Abode& abode) const;
	void contemplate(const Room& room) const;

	/// Whether this witness contemplates the words of this place now: the place
	/// itself, or a room reflecting it.
	bool contemplates(const Place& place) const;

	void say(const Saying& saying) const final;

protected:
	explicit Witness(matter::Man kept);
};


} // namespace will::domain
