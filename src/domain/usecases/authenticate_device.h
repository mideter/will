#pragma once

#include "entities/god.h"
#include "errors/auth_error.h"
#include "ports/god_repository.h"

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
    explicit AuthenticateDevice(GodRepository& gods);

    std::variant<AuthenticateDeviceSuccess, AuthError> execute(const AuthenticateDeviceInput& input);

private:
    GodRepository& gods_;
};


} // namespace will::domain
