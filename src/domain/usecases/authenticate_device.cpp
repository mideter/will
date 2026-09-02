#include "authenticate_device.h"

#include "values/device_token.h"
#include "values/god_name.h"


namespace will::domain {


AuthenticateDevice::AuthenticateDevice(GodRepository& gods) : gods_(gods) {}


std::variant<AuthenticateDeviceSuccess, AuthError> AuthenticateDevice::execute(const AuthenticateDeviceInput& input)
{
    const std::optional<DeviceToken> token = DeviceToken::parse(input.device_token_raw);
    if (!token)
        return AuthError::InvalidToken;

    std::optional<God> god = gods_.find_by_device_token(token->text());
    if (!god)
        god = gods_.create_god(token->text(), GodName::generate());

    return AuthenticateDeviceSuccess{*god};
}


} // namespace will::domain
