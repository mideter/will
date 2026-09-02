#pragma once

#include "values/device_token.h"
#include "ids/god_id.h"
#include "values/god_name.h"


namespace will::domain {


/// God (Бог) — участник обители, привязанный к одному device token.
class God {
public:
    /// god_id must be non-zero (enforced by GodId).
    God(GodId id, DeviceToken device_token, GodName name);

    GodId id() const noexcept { return id_; }
    const DeviceToken& device_token() const noexcept { return device_token_; }
    const GodName& name() const noexcept { return name_; }

    bool operator==(const God&) const = default;

private:
    GodId id_;
    DeviceToken device_token_;
    GodName name_;
};


} // namespace will::domain
