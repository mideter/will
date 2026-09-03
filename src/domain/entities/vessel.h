#pragma once

#include "ids/god.h"
#include "ids/vessel.h"
#include "values/device_token.h"


namespace will::domain {


/// Vessel (Сосуд) — device through which a god reaches the world.
class Vessel {
public:
    Vessel(id::Vessel id, DeviceToken token, id::God god_id);

    id::Vessel id() const noexcept { return id_; }
    const DeviceToken& token() const noexcept { return token_; }
    id::God god_id() const noexcept { return god_id_; }

    bool operator==(const Vessel&) const = default;

private:
    id::Vessel id_;
    DeviceToken token_;
    id::God god_id_;
};


} // namespace will::domain
