#include "sqlite_database.h"
#include "sqlite_message_repository_impl.h"
#include "sqlite_user_repository_impl.h"

#include "entities/chat_id.h"
#include "entities/user_id.h"
#include "support/timestamp.h"

#include <cassert>
#include <cstdlib>
#include <string>
#include <unistd.h>


int main()
{
    using namespace will;
    using namespace will::domain;

    const std::string db_path = "/tmp/will-sqlite-persistence-test-" + std::to_string(getpid()) + ".db";
    ::unlink(db_path.c_str());

    SqliteDatabase database(db_path);
    SqliteMessageRepositoryImpl messages(database);
    SqliteUserRepositoryImpl users(database);

    const User user_a = users.create_user("aaaa1234aaaa1234aaaa1234aaaa1234", "nameaaaa");
    const User user_b = users.create_user("bbbb1234bbbb1234bbbb1234bbbb1234", "namebbbb");
    const ChatId chat = ChatId::global();

    messages.append(chat, user_a.id, "from-peer", *Timestamp::parse(1000));
    messages.append(chat, user_b.id, "from-me", *Timestamp::parse(2000));

    const auto rows = messages.load_last(chat, 10);
    assert(rows.size() == 2);
    assert(rows[0].body == "from-peer");
    assert(rows[0].author_id == user_a.id);
    assert(rows[0].author_name == "nameaaaa");
    assert(rows[1].body == "from-me");
    assert(rows[1].author_id == user_b.id);
    assert(rows[1].author_name == "namebbbb");

    const std::string token = "abcd1234abcd1234abcd1234abcd1234";
    const User created = users.create_user(token, "abcdefgh");
    assert(created.id.value > 0);
    assert(created.device_token == token);
    assert(created.name == "abcdefgh");

    const std::optional<User> found = users.find_by_device_token(token);
    assert(found.has_value());
    assert(found->id == created.id);
    assert(found->name == "abcdefgh");

    users.set_name(created.id, "newname1");
    const std::optional<User> renamed = users.find_by_device_token(token);
    assert(renamed.has_value());
    assert(renamed->name == "newname1");

    ::unlink(db_path.c_str());
    return EXIT_SUCCESS;
}
