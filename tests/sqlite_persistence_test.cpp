#include "sqlite_database.h"
#include "sqlite_message_repository_impl.h"
#include "sqlite_user_repository_impl.h"

#include "ids/chat_id.h"
#include "entities/timestamp.h"
#include "ids/user_id.h"
#include "entities/user_name.h"

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

    std::optional<UserId> user_a_id;
    std::optional<UserId> user_b_id;
    std::optional<UserId> created_id;
    const std::string token = "abcd1234abcd1234abcd1234abcd1234";

    {
        SqliteDatabase database(db_path);
        SqliteMessageRepositoryImpl messages(database);
        SqliteUserRepositoryImpl users(database);

        const User user_a = users.create_user("aaaa1234aaaa1234aaaa1234aaaa1234", *UserName::parse("nameaaaa"));
        const User user_b = users.create_user("bbbb1234bbbb1234bbbb1234bbbb1234", *UserName::parse("namebbbb"));
        user_a_id = user_a.id();
        user_b_id = user_b.id();

        const ChatId chat = ChatId::global();
        messages.append(chat, user_a.id(), "from-peer", Timestamp{1000});
        messages.append(chat, user_b.id(), "from-me", Timestamp{2000});

        const auto rows = messages.load_last(chat, 10);
        assert(rows.size() == 2);
        assert(rows[0].body() == "from-peer");
        assert(rows[0].author_id() == user_a.id());
        assert(users.find_by_id(rows[0].author_id())->name() == *UserName::parse("nameaaaa"));
        assert(rows[1].body() == "from-me");
        assert(rows[1].author_id() == user_b.id());
        assert(users.find_by_id(rows[1].author_id())->name() == *UserName::parse("namebbbb"));

        const User created = users.create_user(token, *UserName::parse("abcdefgh"));
        created_id = created.id();
        assert(created.id().value() > 0);
        assert(created.device_token().text() == token);
        assert(created.name() == *UserName::parse("abcdefgh"));

        const std::optional<User> found = users.find_by_device_token(token);
        assert(found.has_value());
        assert(found->id() == created.id());
        assert(found->name() == *UserName::parse("abcdefgh"));
    }

    {
        SqliteDatabase database(db_path);
        SqliteUserRepositoryImpl users(database);

        const std::optional<User> by_token = users.find_by_device_token(token);
        assert(by_token.has_value());
        assert(by_token->id() == *created_id);
        assert(by_token->name() == *UserName::parse("abcdefgh"));

        const std::optional<User> a = users.find_by_id(*user_a_id);
        assert(a.has_value());
        assert(a->name() == *UserName::parse("nameaaaa"));
        assert(a->device_token().text() == "aaaa1234aaaa1234aaaa1234aaaa1234");

        const std::optional<User> b = users.find_by_id(*user_b_id);
        assert(b.has_value());
        assert(b->name() == *UserName::parse("namebbbb"));

        assert(!users.find_by_id(UserId{999999}).has_value());
        assert(!users.find_by_device_token("missing-token-xxxxxxxxxxxxxxxx").has_value());
    }

    ::unlink(db_path.c_str());
    return EXIT_SUCCESS;
}
