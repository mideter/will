#pragma once

#include "entities/account.h"
#include "entities/auth_token.h"
#include "errors/auth_error.h"
#include "ports/user_repository.h"

#include <string_view>
#include <variant>


namespace will::domain {


struct AuthenticateDeviceInput {
    std::string_view device_token_raw;
    Timestamp now{};
};


struct AuthenticateDeviceSuccess {
    Account account;
};


class AuthenticateDevice {
public:
    explicit AuthenticateDevice(UserRepository& users);

    [[nodiscard]] std::variant<AuthenticateDeviceSuccess, AuthError> execute(const AuthenticateDeviceInput& input);

private:
    UserRepository& users_;
};


} // namespace will::domain
