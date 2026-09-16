#pragma once

#include "entities/man.h"
#include "entities/obedience.h"
#include "entities/supplication.h"
#include "entities/witness.h"
#include "identity/soul.h"


namespace will::domain {


/// Executor (Послушник) — Witness of a will other than his own.
/// Mode is disclosed in an Obedience; the living heap object is always Testator.
class Executor : public Witness {
public:
	/// Ask addressee to become testator; this soul will be the executor.
	Supplication supplicate(id::Soul testator) const;

	/// Leave a living obedience (as either side of the pair).
	void secede(const Obedience& obedience) const;

protected:
	explicit Executor(Man&& man);
};


} // namespace will::domain
