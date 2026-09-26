#pragma once

#include "beings/immanents/place.h"


namespace will::domain {


class Novice;
class Soul;
class Testator;


/// Obedience (Послушание) — Novice-facing interface of a shared Place.
/// Living heap object is Tie (Узы), owned by the Novice. Holds the
/// counterpart side (testator); accessors are completed by Tie. Ending a
/// place (secede) is deferred for now — all kept places are active.
class Obedience : public virtual Place {
public:
	~Obedience() override = default;

	Obedience(const Obedience&) = delete;
	Obedience& operator=(const Obedience&) = delete;

	virtual const Testator& testator() const = 0;
	virtual const Novice& novice() const = 0;

protected:
	explicit Obedience(const Soul& testator);
	Obedience& operator=(Obedience&&) = delete;

	const Testator& testator_;
};


} // namespace will::domain
