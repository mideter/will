#include "authenticate_device.h"

#include "entities/dead_vessel.h"

#include <optional>
#include <stdexcept>
#include <utility>


namespace will::domain {


AuthenticateDevice::AuthenticateDevice(Heaven& heaven, Earth& earth)
    : heaven_(heaven)
    , earth_(earth)
{}


std::variant<AuthenticateDeviceSuccess, AuthError> AuthenticateDevice::execute(
    const AuthenticateDeviceInput& input)
{
    try {
        const DeadVessel dead{input.device_token_raw};

        if (const std::optional<Vessel> vessel = earth_.find_by_dead(dead)) {
            if (const std::optional<God> god = heaven_.find_by_id(vessel->god_id()))
                return AuthenticateDeviceSuccess{*god};
            return AuthError::InvalidToken;
        }

        auto [god, vessel] = heaven_.remember_with_vessel(dead);
        earth_.insert(std::move(vessel));
        return AuthenticateDeviceSuccess{god};
    } catch (const std::invalid_argument&) {
        return AuthError::InvalidToken;
    }
}


} // namespace will::domain
