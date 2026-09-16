#pragma once

#include "entities/abode.h"
#include "entities/man.h"
#include "entities/witness.h"


namespace will::domain {


/// Executor (Послушник) — Witness of a will other than his own.
/// Mode is disclosed in a Bond (later); the living heap object is always Testator.
class Executor : public Witness {
protected:
	Executor(Man&& man, Abode& abode) noexcept;
};


} // namespace will::domain
