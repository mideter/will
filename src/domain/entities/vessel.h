#pragma once

#include "ids/god_id.h"
#include "ids/vessel_id.h"
#include "values/device_token.h"


namespace will::domain {


/// Vessel (Сосуд) — device through which a god reaches the world.
class Vessel {
public:
    Vessel(VesselId id, DeviceToken token, GodId god_id);

    VesselId id() const noexcept { return id_; }
    const DeviceToken& token() const noexcept { return token_; }
    GodId god_id() const noexcept { return god_id_; }

    bool operator==(const Vessel&) const = default;

private:
    VesselId id_;
    DeviceToken token_;
    GodId god_id_;
};


} // namespace will::domain
