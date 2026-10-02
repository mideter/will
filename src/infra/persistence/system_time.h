#pragma once

#include "horizons/time.h"


namespace will {


/// Wall-clock Time — present from the system clock.
class SystemTime final : public domain::Time {
public:
	domain::Timestamp instant() const override;
};


} // namespace will
