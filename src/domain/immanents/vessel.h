#pragma once

#include "immanents/dust.h"
#include "identity/vessel.h"
#include "matter/vessel.h"
#include "values/device_token.h"


namespace will::domain {


/// Vessel (Сосуд) — device through which a soul reaches the world; inherits Dust.
class Vessel : public Dust {
public:
	id::Vessel id() const noexcept { return id_; }
	const DeviceToken& token() const noexcept { return token_; }

	bool operator==(const Vessel& other) const noexcept;

protected:
	explicit Vessel(matter::Vessel kept);

private:
	id::Vessel id_;
	DeviceToken token_;
};


} // namespace will::domain
