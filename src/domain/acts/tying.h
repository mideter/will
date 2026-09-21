#pragma once

#include "identity/soul.h"
#include "identity/tie.h"


namespace will::domain {


/// Tying (Связывание) — pair place recorded in time (testator, novice).
/// Material for living Tie on the heap; not the Tie itself.
class Tying {
public:
	Tying(id::Tie id, id::Soul testator, id::Soul novice);

	id::Tie id() const noexcept { return id_; }
	id::Soul testator() const noexcept { return testator_; }
	id::Soul novice() const noexcept { return novice_; }

private:
	id::Tie id_;
	id::Soul testator_;
	id::Soul novice_;
};


} // namespace will::domain
