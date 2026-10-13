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


/// Witness (Свидетель) — a living man in the waking world: awake, he contemplates one
/// thing, his own abode unless he turns elsewhere; asleep, nothing.
class Witness : public Man {
public:
	void wake() const;

	void sleep() const;

	/// A tie is seen only through the room reflecting it.
	void contemplate(const Abode& abode) const;
	void contemplate(const Room& room) const;

	/// The place itself, or a room reflecting it.
	bool contemplates(const Place& place) const;

	void say(const Saying& saying) const final;

	/// Write where one stands: a letter in one's own word, a behest in a tie one wills in.
	std::shared_ptr<const Word> write(const Saying& saying) const;

	bool writes_here() const;

	/// A letter in one's own word.
	std::shared_ptr<const Letter> record(const Saying& saying) const;

protected:
	explicit Witness(matter::Man kept);
};


} // namespace will::domain
