#pragma once

#include "values/device_token.h"

#include <functional>
#include <string_view>


namespace will::domain {


/// DeadVessel (Мёртвый сосуд) — device token without a soul.
class DeadVessel {
public:
    /// Throws std::invalid_argument if the device token is invalid.
    explicit DeadVessel(std::string_view device_token_raw);

    explicit DeadVessel(DeviceToken token);

    std::string_view text() const noexcept { return token_.text(); }

    bool operator==(const DeadVessel&) const = default;

private:
    DeviceToken token_;
};


} // namespace will::domain


template <>
struct std::hash<will::domain::DeadVessel> {
    std::size_t operator()(const will::domain::DeadVessel& vessel) const noexcept
    {
        return std::hash<std::string_view>{}(vessel.text());
    }
};
