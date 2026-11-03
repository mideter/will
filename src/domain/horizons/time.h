#pragma once

#include "values/timestamp.h"


namespace will::domain {


class Time {
public:
	virtual ~Time() = default;

	virtual Timestamp instant() const = 0;
};


} // namespace will::domain
