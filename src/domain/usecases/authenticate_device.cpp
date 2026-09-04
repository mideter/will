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

	if (std::optional<Man> man = world_.find_man_by_token(*token))
		return AuthenticateDeviceSuccess{std::move(*man)};

	return AuthenticateDeviceSuccess{world_.remember_man(*token)};
}


} // namespace will::domain
