#pragma once

#include "acts/obedience.h"
#include "acts/supplication.h"
#include "entities/witness.h"
#include "identity/man.h"
#include "identity/soul.h"
#include "identity/vessel.h"
#include "values/device_token.h"
#include "values/soul_name.h"


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
	Executor(id::Man id, id::Soul soul_id, SoulName name, id::Vessel vessel_id, DeviceToken token);
};


} // namespace will::domain
