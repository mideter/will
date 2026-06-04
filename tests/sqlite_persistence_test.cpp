#include "sqlite_auth_session_store_impl.h"
#include "sqlite_database.h"
#include "sqlite_message_repository_impl.h"
#include "sqlite_user_repository_impl.h"

#include "entities/chat_id.h"
#include "entities/user_id.h"
#include "usecases/authenticate_user.h"

#include <cassert>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <variant>


int main()
{
    using namespace will;
    using namespace will::domain;

    const std::string db_path = "/tmp/will-sqlite-persistence-test-" + std::to_string(getpid()) + ".db";
    ::unlink(db_path.c_str());

    SqliteDatabase database(db_path);
    SqliteMessageRepositoryImpl messages(database);
    SqliteUserRepositoryImpl users(database);
    SqliteAuthSessionStoreImpl sessions(database);

    const UserId peer_a{42};
    const UserId peer_b{99};
    const ChatId chat = ChatId::global();

    messages.append(chat, peer_a, "from-peer", 1000);
    messages.append(chat, peer_b, "from-me", 2000);

    const auto rows = messages.load_last(chat, 10);
    assert(rows.size() == 2);
    assert(rows[0].body == "from-peer");
    assert(rows[0].author_id == peer_a);
    assert(rows[1].body == "from-me");
    assert(rows[1].author_id == peer_b);

    const std::optional<User> admin = users.find_by_login("admin");
    assert(admin.has_value());
    assert(admin->login == "admin");
    assert(users.verify_password(admin->id, "admin"));
    assert(!users.verify_password(admin->id, "wrong"));

    AuthenticateUser authenticate(users, sessions);
    const auto ok = authenticate.execute(AuthenticateUserInput{"admin", "admin", 3000});
    assert(std::holds_alternative<AuthenticateUserSuccess>(ok));
    const Account& account = std::get<AuthenticateUserSuccess>(ok).account;
    assert(account.user_id == admin->id);
    assert(!account.session_token.empty());

    const std::optional<Account> resolved = sessions.resolve_token(account.session_token);
    assert(resolved.has_value());
    assert(resolved->user_id == admin->id);

    ::unlink(db_path.c_str());
    return EXIT_SUCCESS;
}
