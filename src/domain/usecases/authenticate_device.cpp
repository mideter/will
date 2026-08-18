module;

#include <optional>
#include <string>
#include <variant>

module will.domain.authenticate_device;

import will.domain.device_token;
import will.domain.user_name;


namespace will::domain {


AuthenticateDevice::AuthenticateDevice(UserRepository& users) : users_(users) {}


std::variant<AuthenticateDeviceSuccess, AuthError> AuthenticateDevice::execute(const AuthenticateDeviceInput& input)
{
    const std::optional<DeviceToken> token = DeviceToken::parse(input.device_token_raw);
    if (!token)
        return AuthError::InvalidToken;

    std::optional<User> user = users_.find_by_device_token(token->text());
    if (!user) {
        user = users_.create_user(token->text(), UserName::generate());
    } else if (user->name.empty()) {
        const std::string name = UserName::generate();
        users_.set_name(user->id, name);
        user->name = name;
    }

    return AuthenticateDeviceSuccess{Account{user->id, token->value(), input.now, user->name}};
}


} // namespace will::domain
