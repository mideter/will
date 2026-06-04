#pragma once

#include "entities/account.h"
#include "entities/auth_token.h"
#include "entities/user_id.h"

#include <optional>


namespace will::domain {


class AuthSessionStore {
public:
    virtual ~AuthSessionStore() = default;

    virtual AuthToken issue_session(UserId user) = 0;
    virtual std::optional<Account> resolve_token(AuthToken token) = 0;
    virtual void revoke(AuthToken token) = 0;
};


} // namespace will::domain
