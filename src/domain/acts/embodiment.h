#pragma once

#include "identity/soul.h"
#include "identity/vessel.h"
#include "values/device_token.h"
#include "values/soul_name.h"


namespace will::domain {


/// Embodiment (Воплощение) — soul dwelling recorded in time (vessel binding).
/// Material for World birth; not a living Man. One soul, one living man in the World.
class Embodiment {
public:
	Embodiment(id::Soul soul, SoulName name, id::Vessel vessel, DeviceToken token);

	id::Soul soul() const noexcept { return soul_; }
	const SoulName& name() const noexcept { return name_; }
	id::Vessel vessel() const noexcept { return vessel_; }
	const DeviceToken& token() const noexcept { return token_; }

private:
	id::Soul soul_;
	SoulName name_;
	id::Vessel vessel_;
	DeviceToken token_;
};


} // namespace will::domain
