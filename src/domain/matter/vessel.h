#pragma once

#include "identity/vessel.h"
#include "values/device_token.h"


namespace will::domain::matter {


/// Matter of living Vessel (Сосуд, the body) — its id and device token.
/// Temporality keeps it; whose body it is, the Embodiment tells.
class Vessel {
public:
	Vessel(id::Vessel id, DeviceToken token);

	id::Vessel id() const noexcept { return id_; }
	const DeviceToken& token() const noexcept { return token_; }

private:
	id::Vessel id_;
	DeviceToken token_;
};


} // namespace will::domain::matter
