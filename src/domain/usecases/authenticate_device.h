#pragma once

#include "entities/user.h"
#include "errors/auth_error.h"
#include "ports/user_repository.h"

#include <string_view>
#include <variant>


namespace will::domain {


struct AuthenticateDeviceInput {
    std::string_view device_token_raw;
};


struct AuthenticateDeviceSuccess {
    User user;
};


class AuthenticateDevice {
public:
    explicit AuthenticateDevice(UserRepository& users);

    std::variant<AuthenticateDeviceSuccess, AuthError> execute(const AuthenticateDeviceInput& input);

private:
    UserRepository& users_;
};


} // namespace will::domain
