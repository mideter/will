#pragma once

#include "identity/soul.h"


namespace will::domain::matter {


/// Fatherhood (Отцовство) — a father and his child, by flesh or by spirit.
/// The father by flesh is the host of the gates the child was born at: one,
/// never changed. The father by spirit the child chooses: at most one at a time.
/// Eternity keeps it, for it binds souls.
class Fatherhood {
public:
	enum class Line {
		Flesh,
		Spirit,
	};

	Fatherhood(id::Soul father, id::Soul child, Line line);

	id::Soul father() const noexcept { return father_; }
	id::Soul child() const noexcept { return child_; }
	Line line() const noexcept { return line_; }

private:
	id::Soul father_;
	id::Soul child_;
	Line line_;
};


} // namespace will::domain::matter
