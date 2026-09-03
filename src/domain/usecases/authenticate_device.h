#pragma once

#include "errors/auth_error.h"

#include "entities/earth.h"
#include "entities/god.h"

#include <string_view>
#include <variant>


namespace will::domain {


struct AuthenticateDeviceInput {
    std::string_view device_token_raw;
};


struct AuthenticateDeviceSuccess {
    God god;
};


class AuthenticateDevice {
public:
    explicit AuthenticateDevice(Earth& earth);

    std::variant<AuthenticateDeviceSuccess, AuthError> execute(const AuthenticateDeviceInput& input);

private:
    Earth& earth_;
};


} // namespace will::domain
