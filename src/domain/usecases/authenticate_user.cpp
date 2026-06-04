#include "authenticate_user.h"


namespace will::domain {


AuthenticateUser::AuthenticateUser(UserRepository& users, AuthSessionStore& sessions)
    : users_(users)
    , sessions_(sessions)
{}


std::variant<AuthenticateUserSuccess, AuthResult> AuthenticateUser::execute(const AuthenticateUserInput& input)
{
    const std::optional<User> user = users_.find_by_login(input.login);

    if (!user)
        return AuthResult::InvalidCredentials;

    if (!users_.verify_password(user->id, input.password))
        return AuthResult::InvalidCredentials;

    const AuthToken token = sessions_.issue_session(user->id);
    Account account{user->id, token, input.authenticated_at};
    
    return AuthenticateUserSuccess{std::move(account)};
}


} // namespace will::domain
