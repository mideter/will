#pragma once

#include "matter/man.h"
#include "places/abode.h"
#include "men/man.h"
#include "values/saying.h"

#include <memory>


namespace will::domain {


class Room;
class Letter;
class Word;


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

	/// Write where one stands: the place takes it as its words are taken — a letter
	/// in one's own word, a behest in a tie one wills in — or the world refuses,
	/// before anything is kept. The word it became.
	std::shared_ptr<const Word> write(const Saying& saying) const;

	/// Whether one may write where one stands now.
	bool writes_here() const;

	/// Record in one's own word: a letter. Refuses elsewhere.
	std::shared_ptr<const Letter> record(const Saying& saying) const;

protected:
	explicit Witness(matter::Man kept);
};


} // namespace will::domain
