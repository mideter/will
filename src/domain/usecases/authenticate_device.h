#pragma once

#include "errors/auth_error.h"

#include "entities/earth.h"
#include "entities/man.h"
#include "entities/heaven.h"

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
    AuthenticateDevice(Heaven& heaven, Earth& earth);

    std::variant<AuthenticateDeviceSuccess, AuthError> execute(const AuthenticateDeviceInput& input);

private:
    Heaven& heaven_;
    Earth& earth_;
};


} // namespace will::domain
