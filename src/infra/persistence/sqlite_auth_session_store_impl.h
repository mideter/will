#pragma once

#include "ports/auth_session_store.h"
#include "sqlite_database.h"


namespace will {


class SqliteAuthSessionStoreImpl final : public domain::AuthSessionStore {
public:
    explicit SqliteAuthSessionStoreImpl(SqliteDatabase& database);

    domain::AuthToken issue_session(domain::UserId user) override;
    std::optional<domain::Account> resolve_token(domain::AuthToken token) override;
    void revoke(domain::AuthToken token) override;

private:
    SqliteDatabase& database_;
};


} // namespace will
