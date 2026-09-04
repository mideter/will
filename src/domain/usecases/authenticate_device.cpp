#include "authenticate_device.h"

#include "values/device_token.h"

#include <optional>
#include <utility>


namespace will::domain {


AuthenticateDevice::AuthenticateDevice(Heaven& heaven, Earth& earth)
    : heaven_(heaven)
    , earth_(earth)
{}


std::variant<AuthenticateDeviceSuccess, AuthError> AuthenticateDevice::execute(
    const AuthenticateDeviceInput& input)
{
    const std::optional<DeviceToken> token = DeviceToken::parse(input.device_token_raw);
    if (!token)
        return AuthError::InvalidToken;

    if (std::optional<Man> man = earth_.find_man_by_token(*token))
        return AuthenticateDeviceSuccess{std::move(*man)};

    ManBirth birth = heaven_.remember_man(*token);
    earth_.insert(std::move(birth.vessel));
    earth_.insert(birth.man);
    return AuthenticateDeviceSuccess{std::move(birth.man)};
}


} // namespace will::domain
