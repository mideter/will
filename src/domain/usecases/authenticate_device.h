#pragma once

#include "entities/god.h"
#include "errors/auth_error.h"
#include "ports/heaven.h"

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
    explicit AuthenticateDevice(Heaven& heaven);

    std::variant<AuthenticateDeviceSuccess, AuthError> execute(const AuthenticateDeviceInput& input);

private:
    Heaven& heaven_;
};


} // namespace will::domain
