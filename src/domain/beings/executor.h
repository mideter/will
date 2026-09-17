#pragma once

#include "acts/embodiment.h"
#include "acts/obedience.h"
#include "acts/supplication.h"
#include "beings/witness.h"


namespace will::domain {


class Soul;


/// Executor (Послушник) — Witness of a will other than his own.
/// Mode is disclosed in an Obedience; the living heap object is always Testator.
class Executor : public Witness {
public:
	/// Ask addressee to become testator; this soul will be the executor.
	Supplication supplicate(const Soul& testator) const;

	/// Leave a living obedience (as either side of the pair).
	void secede(const Obedience& obedience) const;

protected:
	explicit Executor(Embodiment embodiment);
};


} // namespace will::domain
