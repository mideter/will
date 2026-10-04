#pragma once

#include "identity/word.h"


namespace will::domain::matter {


/// Execution (Исполнение) — this deed fulfils that behest; part of the matter of
/// a Deed, with no living pair of its own. Temporality keeps Executions; a
/// behest is fulfilled at most once.
class Execution {
public:
	Execution(id::Word deed, id::Word behest);

	/// The word of the deed — the same id as the other parts of its matter.
	id::Word id() const noexcept { return deed_; }
	id::Word behest() const noexcept { return behest_; }

private:
	id::Word deed_;
	id::Word behest_;
};


} // namespace will::domain::matter
