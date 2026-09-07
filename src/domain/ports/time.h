#pragma once

#include "values/timestamp.h"


namespace will::domain {


/// Time — present given through Eternity (and thus through Temporality).
class Time {
public:
	virtual ~Time() = default;

	/// The present instant.
	virtual Timestamp instant() const = 0;
};


} // namespace will::domain
