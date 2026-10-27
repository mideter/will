#pragma once

#include "men/dust.h"
#include "identity/vessel.h"
#include "matter/vessel.h"
#include "values/device_token.h"


namespace will::domain {


/// The device through which a soul reaches the world.
class Vessel : public Dust {
public:
	virtual ~Vessel() = default;

	id::Vessel id() const noexcept { return id_; }
	const DeviceToken& token() const noexcept { return token_; }


protected:
	explicit Vessel(matter::Vessel kept);

private:
	id::Vessel id_;
	DeviceToken token_;
};


} // namespace will::domain
