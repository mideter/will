#pragma once

#include "values/timestamp.h"


namespace will::domain {


/// Time — the one present, given only through Eternity.
class Time {
public:
	virtual ~Time() = default;

	/// The present instant.
	virtual Timestamp instant() const = 0;
};


} // namespace will::domain
