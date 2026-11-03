#pragma once

#include "identity/soul.h"


namespace will::domain::matter {


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
