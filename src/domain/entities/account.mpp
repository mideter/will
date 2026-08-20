module;

#include <string>

export module will.domain.account;

export import will.domain.auth_token;
export import will.domain.timestamp;
export import will.domain.user_id;

export namespace will::domain {


/// Authenticated connection context for an active session.
struct Account {
    UserId user_id{};
    AuthToken session_token;
    Timestamp authenticated_at{};
    std::string name;
};


} // namespace will::domain
