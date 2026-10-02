#pragma once

#include "identity/soul.h"
#include "identity/tie.h"


namespace will::domain::matter {


/// Matter of living Tie (Узы) — pair place kept in space (testator, novice).
/// Attests the bond; not the living Tie itself.
class Tie {
public:
	Tie(id::Tie id, id::Soul testator, id::Soul novice);

	id::Tie id() const noexcept { return id_; }
	id::Soul testator() const noexcept { return testator_; }
	id::Soul novice() const noexcept { return novice_; }

private:
	id::Tie id_;
	id::Soul testator_;
	id::Soul novice_;
};


} // namespace will::domain::matter
