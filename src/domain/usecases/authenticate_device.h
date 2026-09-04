#pragma once

#include "errors/auth_error.h"

#include "entities/man.h"
#include "entities/world.h"

#include <string_view>
#include <variant>


namespace will::domain {


struct AuthenticateDeviceInput {
	std::string_view device_token_raw;
};


struct AuthenticateDeviceSuccess {
	Man man;
};


class AuthenticateDevice {
public:
	explicit AuthenticateDevice(World& world);

	std::variant<AuthenticateDeviceSuccess, AuthError> execute(const AuthenticateDeviceInput& input);

private:
	World& world_;
};


} // namespace will::domain
