#pragma once

#include "entities/account.h"
#include "entities/message.h"
#include "errors/domain_error.h"
#include "ports/auth_session_store.h"
#include "ports/user_repository.h"

#include <string_view>
#include <variant>


namespace will::domain {


struct AuthenticateUserInput {
    std::string_view login;
    std::string_view password;
    TimestampMs authenticated_at = 0;
};


struct AuthenticateUserSuccess {
    Account account;
};


class AuthenticateUser {
public:
    AuthenticateUser(UserRepository& users, AuthSessionStore& sessions);

    std::variant<AuthenticateUserSuccess, AuthResult> execute(const AuthenticateUserInput& input);

private:
    UserRepository& users_;
    AuthSessionStore& sessions_;
};


} // namespace will::domain
