#include "domain_fakes.h"

#include "entities/chat_id.h"
#include "entities/participant_id.h"
#include "errors/auth_error.h"
#include "errors/domain_error.h"
#include "support/device_token.h"
#include "usecases/authenticate_device.h"
#include "usecases/fetch_chat_history.h"
#include "usecases/send_chat_message.h"

#include <cassert>
#include <cstdlib>
#include <variant>


namespace {


using namespace will::domain;
using namespace will::domain::test;


void test_authenticate_device_creates_user()
{
    FakeUserRepository users;
    AuthenticateDevice authenticate(users);

    const AuthToken token = DeviceToken::generate();
    const auto result = authenticate.execute(AuthenticateDeviceInput{token.value, 1000});
    assert(std::holds_alternative<AuthenticateDeviceSuccess>(result));

    const AuthenticateDeviceSuccess& success = std::get<AuthenticateDeviceSuccess>(result);
    assert(success.account.user_id.value > 0);
    assert(success.account.session_token == token);

    const std::optional<User> user = users.find_by_device_token(token.value);
    assert(user.has_value());
    assert(user->id == success.account.user_id);
}


void test_authenticate_device_existing_user()
{
    FakeUserRepository users;
    users.add_user(User{UserId{42}, "abcd1234abcd1234abcd1234abcd1234"});

    AuthenticateDevice authenticate(users);
    const auto result = authenticate.execute(
        AuthenticateDeviceInput{"abcd1234abcd1234abcd1234abcd1234", 1000});
    assert(std::holds_alternative<AuthenticateDeviceSuccess>(result));
    assert(std::get<AuthenticateDeviceSuccess>(result).account.user_id == UserId{42});
}


void test_authenticate_device_invalid_token()
{
    FakeUserRepository users;
    AuthenticateDevice authenticate(users);

    const auto result = authenticate.execute(AuthenticateDeviceInput{"short", 1000});
    assert(std::holds_alternative<AuthError>(result));
    assert(std::get<AuthError>(result) == AuthError::InvalidToken);
}


void test_send_chat_message_persists_and_notifies()
{
    InMemoryMessageRepository messages;
    FakeParticipantNotifier notifier;

    const Account account{UserId{7}, AuthToken{"tok"}, 500};
    const ParticipantId sender{42};

    SendChatMessage send(messages, notifier);
    const Message saved = send.execute(SendChatMessageInput{account, sender, ChatId::global(), "hello", 900});

    assert(saved.id > 0);
    assert(saved.author_id == account.user_id);
    assert(saved.body == "hello");
    assert(saved.created_at == 900);

    const auto loaded = messages.load_last(ChatId::global(), 10);
    assert(loaded.size() == 1);
    assert(loaded[0].body == "hello");

    assert(notifier.notifications_.size() == 1);
    assert(notifier.notifications_[0].except == sender);
    assert(notifier.notifications_[0].message.id == saved.id);
}


void test_fetch_chat_history_limit_and_is_mine()
{
    InMemoryMessageRepository messages;
    const ChatId chat = ChatId::global();
    const UserId me{10};
    const UserId other{20};

    messages.append(chat, other, "peer", 1);
    messages.append(chat, me, "mine", 2);
    messages.append(chat, other, "peer2", 3);

    FetchChatHistory fetch(messages);

    const auto zero_limit = fetch.execute(FetchChatHistoryInput{Account{me, {}, 0}, chat, 0});
    assert(std::holds_alternative<DomainError>(zero_limit));
    assert(std::get<DomainError>(zero_limit).code == DomainErrorCode::InvalidArgument);

    const auto ok = fetch.execute(FetchChatHistoryInput{Account{me, {}, 0}, chat, 2});
    assert(std::holds_alternative<FetchChatHistoryResult>(ok));

    const FetchChatHistoryResult& result = std::get<FetchChatHistoryResult>(ok);
    assert(result.items.size() == 2);
    assert(result.items[0].message.body == "mine");
    assert(result.items[0].is_mine);
    assert(result.items[1].message.body == "peer2");
    assert(!result.items[1].is_mine);
}


void test_fetch_chat_history_caps_limit()
{
    InMemoryMessageRepository messages;
    const ChatId chat = ChatId::global();
    const UserId author{1};

    for (int i = 0; i < 5; ++i)
        messages.append(chat, author, "m", static_cast<TimestampMs>(i));

    FetchChatHistory fetch(messages);
    const auto ok = fetch.execute(
        FetchChatHistoryInput{Account{author, {}, 0}, chat, FetchChatHistory::MaxHistoryRequestLimit + 50});
    assert(std::holds_alternative<FetchChatHistoryResult>(ok));
    assert(std::get<FetchChatHistoryResult>(ok).items.size() == 5);
}


} // namespace


int main()
{
    test_authenticate_device_creates_user();
    test_authenticate_device_existing_user();
    test_authenticate_device_invalid_token();
    test_send_chat_message_persists_and_notifies();
    test_fetch_chat_history_limit_and_is_mine();
    test_fetch_chat_history_caps_limit();
    return EXIT_SUCCESS;
}
