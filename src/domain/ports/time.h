#pragma once

#include "values/timestamp.h"


namespace will::domain {


/// Time — present given through Eternity; Temporality receives it to fix moments.
class Time {
public:
	virtual ~Time() = default;

	/// The present instant.
	virtual Timestamp instant() const = 0;
};


} // namespace will::domain
