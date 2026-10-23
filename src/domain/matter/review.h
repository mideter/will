#pragma once

#include "identity/word.h"
#include "values/timestamp.h"


namespace will::domain::matter {


/// The testator saw this deed, then: it is reviewed once.
class Review {
public:
	Review(id::Word deed, Timestamp at);

	id::Word id() const noexcept { return deed_; }
	Timestamp at() const noexcept { return at_; }

private:
	id::Word deed_;
	Timestamp at_;
};


} // namespace will::domain::matter
