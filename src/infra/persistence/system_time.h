#pragma once

#include "horizons/time.h"


namespace will {


class SystemTime final : public domain::Time {
public:
	domain::Timestamp instant() const override;
};


} // namespace will
