#include "authenticate_device.h"

#include <optional>


namespace will::domain {


AuthenticateDevice::AuthenticateDevice(Earth& earth)
    : earth_(earth)
{}


std::variant<AuthenticateDeviceSuccess, AuthError> AuthenticateDevice::execute(
    const AuthenticateDeviceInput& input)
{
    if (const std::optional<God> god = earth_.receive(input.device_token_raw))
        return AuthenticateDeviceSuccess{*god};

    return AuthError::InvalidToken;
}


} // namespace will::domain
