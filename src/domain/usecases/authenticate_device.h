#pragma once

#include "errors/auth_error.h"

#include "entities/earth.h"
#include "entities/heaven.h"
#include "ports/eternity.h"

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
    AuthenticateDevice(Heaven& heaven, Earth& earth, Eternity& eternity);

    std::variant<AuthenticateDeviceSuccess, AuthError> execute(const AuthenticateDeviceInput& input);

private:
    Heaven& heaven_;
    Earth& earth_;
    Eternity& eternity_;
};


} // namespace will::domain
