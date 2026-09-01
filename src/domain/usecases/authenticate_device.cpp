#include "authenticate_device.h"

#include "entities/device_token.h"
#include "entities/user_name.h"


namespace will::domain {


AuthenticateDevice::AuthenticateDevice(UserRepository& users) : users_(users) {}


std::variant<AuthenticateDeviceSuccess, AuthError> AuthenticateDevice::execute(const AuthenticateDeviceInput& input)
{
    const std::optional<DeviceToken> token = DeviceToken::parse(input.device_token_raw);
    if (!token)
        return AuthError::InvalidToken;

    std::optional<User> user = users_.find_by_device_token(token->text());
    if (!user)
        user = users_.create_user(token->text(), UserName::generate());

    return AuthenticateDeviceSuccess{Account{user->id(), *token, input.now}};
}


} // namespace will::domain
