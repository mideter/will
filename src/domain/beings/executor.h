#pragma once

#include "acts/embodiment.h"
#include "acts/obedience.h"
#include "acts/supplication.h"
#include "beings/deed.h"
#include "beings/witness.h"


namespace will::domain {


class Soul;


/// Executor (Послушник) — Исполнитель: исполняет Дело в Послушании.
/// Mode is disclosed in an Obedience; the living heap object is always Testator.
class Executor : public Witness {
public:
	/// Ask addressee to become testator; this soul will be the executor.
	Supplication supplicate(const Soul& testator) const;

	/// Carry out an open deed in a living obedience where this soul is executor.
	Deed execute(const Deed& deed) const;

	/// Leave a living obedience (as either side of the pair).
	void secede(const Obedience& obedience) const;

protected:
	explicit Executor(Embodiment embodiment);

private:
	using Witness::say;
};


} // namespace will::domain
