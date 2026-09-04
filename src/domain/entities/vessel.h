#pragma once

#include "identity/vessel.h"
#include "values/device_token.h"


namespace will::domain {


/// Vessel (Сосуд) — device through which a soul reaches the world.
class Vessel {
public:
    Vessel(id::Vessel id, DeviceToken token);

    id::Vessel id() const noexcept { return id_; }
    const DeviceToken& token() const noexcept { return token_; }

    bool operator==(const Vessel&) const = default;

private:
    id::Vessel id_;
    DeviceToken token_;
};


} // namespace will::domain
