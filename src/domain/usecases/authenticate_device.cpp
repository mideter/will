#include "authenticate_device.h"

#include "values/device_token.h"
#include "values/god_name.h"


namespace will::domain {


AuthenticateDevice::AuthenticateDevice(Heaven& heaven, Earth& earth, Eternity& eternity)
    : heaven_(heaven)
    , earth_(earth)
    , eternity_(eternity)
{}


std::variant<AuthenticateDeviceSuccess, AuthError> AuthenticateDevice::execute(const AuthenticateDeviceInput& input)
{
    const std::optional<DeviceToken> token = DeviceToken::parse(input.device_token_raw);
    if (!token)
        return AuthError::InvalidToken;

    if (const std::optional<id::God> god_id = earth_.god_id_for_token(token->text())) {
        if (const std::optional<God> god = heaven_.find_by_id(*god_id))
            return AuthenticateDeviceSuccess{*god};
    }

    auto [god, vessel] = eternity_.insert_god_with_vessel(token->text(), GodName::generate());
    heaven_.insert(god);
    earth_.insert(std::move(vessel));
    return AuthenticateDeviceSuccess{god};
}


} // namespace will::domain
