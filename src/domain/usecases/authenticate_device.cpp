#include "authenticate_device.h"

#include "values/device_token.h"

#include <optional>
#include <utility>


namespace will::domain {


AuthenticateDevice::AuthenticateDevice(World& world)
	: world_(world)
{}


std::variant<AuthenticateDeviceSuccess, AuthError> AuthenticateDevice::execute(
	const AuthenticateDeviceInput& input)
{
	const std::optional<DeviceToken> token = DeviceToken::parse(input.device_token_raw);
	if (!token)
		return AuthError::InvalidToken;

	return AuthenticateDeviceSuccess{world_.welcome(*token)};
}


} // namespace will::domain
