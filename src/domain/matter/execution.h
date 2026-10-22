#pragma once

#include "identity/word.h"


namespace will::domain::matter {


/// This deed fulfils that behest, at most once.
class Execution {
public:
	Execution(id::Word deed, id::Word behest);

	id::Word id() const noexcept { return deed_; }
	id::Word behest() const noexcept { return behest_; }

private:
	id::Word deed_;
	id::Word behest_;
};


} // namespace will::domain::matter
