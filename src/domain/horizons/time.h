#pragma once

#include "values/timestamp.h"


namespace will::domain {


/// The one present, given only through Eternity.
class Time {
public:
	virtual ~Time() = default;

	virtual Timestamp instant() const = 0;
};


} // namespace will::domain
