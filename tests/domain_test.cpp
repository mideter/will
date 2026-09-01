#include "domain_fakes.h"

#include "entities/chat_id.h"
#include "entities/device_token.h"
#include "entities/participant_id.h"
#include "entities/timestamp.h"
#include "entities/user_name.h"
#include "errors/auth_error.h"
#include "errors/domain_error.h"
#include "usecases/authenticate_device.h"
#include "usecases/fetch_chat_history.h"
#include "usecases/send_chat_message.h"

#include <cassert>
#include <cstdlib>
#include <variant>


namespace {


using namespace will::domain;
using namespace will::domain::test;


DeviceToken test_token(const char* hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
{
    return *DeviceToken::parse(hex);
}


UserName test_name(const char* text)
{
    return *UserName::parse(text);
}


void test_authenticate_device_creates_user()
{
    FakeUserRepository users;
    AuthenticateDevice authenticate(users);

    const DeviceToken token = DeviceToken::generate();
    const auto result = authenticate.execute(AuthenticateDeviceInput{token.text(), Timestamp{1000}});
    assert(std::holds_alternative<AuthenticateDeviceSuccess>(result));

    const AuthenticateDeviceSuccess& success = std::get<AuthenticateDeviceSuccess>(result);
    assert(success.account.user_id.value > 0);
    assert(success.account.device_token == token);
    assert(UserName::parse(success.account.name.text()));

    const std::optional<User> user = users.find_by_device_token(token.text());
    assert(user.has_value());
    assert(user->id == success.account.user_id);
    assert(user->name == success.account.name);
}


void test_authenticate_device_existing_user()
{
    FakeUserRepository users;
    users.add_user(User{UserId{42}, "abcd1234abcd1234abcd1234abcd1234", test_name("oldname1")});

    AuthenticateDevice authenticate(users);
    const auto result = authenticate.execute(
        AuthenticateDeviceInput{"abcd1234abcd1234abcd1234abcd1234", Timestamp{1000}});
    assert(std::holds_alternative<AuthenticateDeviceSuccess>(result));
    assert(std::get<AuthenticateDeviceSuccess>(result).account.user_id == UserId{42});
    assert(std::get<AuthenticateDeviceSuccess>(result).account.name == test_name("oldname1"));
}


void test_authenticate_device_keeps_existing_name()
{
    FakeUserRepository users;
    users.add_user(User{UserId{7}, "abcd1234abcd1234abcd1234abcd1234", test_name("keptname")});

    AuthenticateDevice authenticate(users);
    const auto result = authenticate.execute(
        AuthenticateDeviceInput{"abcd1234abcd1234abcd1234abcd1234", Timestamp{1000}});
    assert(std::holds_alternative<AuthenticateDeviceSuccess>(result));

    const std::optional<User> user = users.find_by_device_token("abcd1234abcd1234abcd1234abcd1234");
    assert(user.has_value());
    assert(user->name == test_name("keptname"));
}


void test_authenticate_device_invalid_token()
{
    FakeUserRepository users;
    AuthenticateDevice authenticate(users);

    const auto result = authenticate.execute(AuthenticateDeviceInput{"short", Timestamp{1000}});
    assert(std::holds_alternative<AuthError>(result));
    assert(std::get<AuthError>(result) == AuthError::InvalidToken);
}


void test_send_chat_message_persists_and_notifies()
{
    InMemoryMessageRepository messages;
    FakeParticipantNotifier notifier;

    const Account account{UserId{7}, test_token(), Timestamp{500}, test_name("sendname")};
    const ParticipantId sender{42};

    SendChatMessage send(messages, notifier);
    const Message saved =
        send.execute(SendChatMessageInput{account, sender, ChatId::global(), "hello", Timestamp{900}});

    assert(saved.id().value() > 0);
    assert(saved.author_id() == account.user_id);
    assert(saved.body() == "hello");
    assert(saved.created_at() == Timestamp{900});

    const auto loaded = messages.load_last(ChatId::global(), 10);
    assert(loaded.size() == 1);
    assert(loaded[0].body() == "hello");

    assert(notifier.notifications_.size() == 1);
    assert(notifier.notifications_[0].except == sender);
    assert(notifier.notifications_[0].message.id() == saved.id());
}


void test_fetch_chat_history_limit_and_is_mine()
{
    InMemoryMessageRepository messages;
    FakeUserRepository users;
    const ChatId chat = ChatId::global();
    const UserId me{10};
    const UserId other{20};

    users.add_user(User{me, "token-me-xxxxxxxxxxxxxxxxxxxx", test_name("menameaa")});
    users.add_user(User{other, "token-other-xxxxxxxxxxxxxxxxx", test_name("peername")});

    messages.append(chat, other, "peer", Timestamp{1});
    messages.append(chat, me, "mine", Timestamp{2});
    messages.append(chat, other, "peer2", Timestamp{3});

    FetchChatHistory fetch(messages, users);
    const Account account{me, test_token(), Timestamp{}, test_name("unused01")};

    const auto zero_limit = fetch.execute(FetchChatHistoryInput{account, chat, 0});
    assert(std::holds_alternative<DomainError>(zero_limit));
    assert(std::get<DomainError>(zero_limit).code == DomainErrorCode::InvalidArgument);

    const auto ok = fetch.execute(FetchChatHistoryInput{account, chat, 2});
    assert(std::holds_alternative<FetchChatHistoryResult>(ok));

    const FetchChatHistoryResult& result = std::get<FetchChatHistoryResult>(ok);
    assert(result.items.size() == 2);
    assert(result.items[0].message.body() == "mine");
    assert(result.items[0].author_name == "menameaa");
    assert(result.items[0].is_mine);
    assert(result.items[1].message.body() == "peer2");
    assert(result.items[1].author_name == "peername");
    assert(!result.items[1].is_mine);
}


void test_fetch_chat_history_caps_limit()
{
    InMemoryMessageRepository messages;
    FakeUserRepository users;
    const ChatId chat = ChatId::global();
    const UserId author{1};

    users.add_user(User{author, "token-author-xxxxxxxxxxxxxxx", test_name("authoraa")});

    for (int i = 0; i < 5; ++i)
        messages.append(chat, author, "m", Timestamp{i});

    FetchChatHistory fetch(messages, users);
    const auto ok = fetch.execute(FetchChatHistoryInput{
        Account{author, test_token(), Timestamp{}, test_name("unused02")}, chat, FetchChatHistory::MaxHistoryRequestLimit + 50});
    assert(std::holds_alternative<FetchChatHistoryResult>(ok));
    assert(std::get<FetchChatHistoryResult>(ok).items.size() == 5);
}


} // namespace


int main()
{
    test_authenticate_device_creates_user();
    test_authenticate_device_existing_user();
    test_authenticate_device_keeps_existing_name();
    test_authenticate_device_invalid_token();
    test_send_chat_message_persists_and_notifies();
    test_fetch_chat_history_limit_and_is_mine();
    test_fetch_chat_history_caps_limit();
    return EXIT_SUCCESS;
}
