#include "domain_fakes.h"

#include "entities/chat_id.h"
#include "entities/participant_id.h"
#include "errors/domain_error.h"
#include "usecases/authenticate_user.h"
#include "usecases/fetch_chat_history.h"
#include "usecases/send_chat_message.h"

#include <cassert>
#include <cstdlib>
#include <variant>


namespace {


using namespace will::domain;
using namespace will::domain::test;


void test_authenticate_user_success()
{
    FakeUserRepository users;
    FakeAuthSessionStore sessions;
    const UserId alice_id{1};
    users.add_user(User{alice_id, "alice", "Alice"}, "secret");

    AuthenticateUser auth(users, sessions);
    const auto result = auth.execute(AuthenticateUserInput{"alice", "secret", 1000});
    assert(std::holds_alternative<AuthenticateUserSuccess>(result));

    const Account& account = std::get<AuthenticateUserSuccess>(result).account;
    assert(account.user_id == alice_id);
    assert(!account.session_token.empty());
    assert(account.authenticated_at == 1000);

    const std::optional<Account> resolved = sessions.resolve_token(account.session_token);
    assert(resolved.has_value());
    assert(resolved->user_id == alice_id);
}


void test_authenticate_user_invalid_credentials()
{
    FakeUserRepository users;
    FakeAuthSessionStore sessions;
    users.add_user(User{UserId{2}, "bob", "Bob"}, "pass");

    AuthenticateUser auth(users, sessions);

    const auto unknown = auth.execute(AuthenticateUserInput{"nobody", "pass", 0});
    assert(std::holds_alternative<AuthResult>(unknown));
    assert(std::get<AuthResult>(unknown) == AuthResult::InvalidCredentials);

    const auto wrong_pass = auth.execute(AuthenticateUserInput{"bob", "wrong", 0});
    assert(std::holds_alternative<AuthResult>(wrong_pass));
    assert(std::get<AuthResult>(wrong_pass) == AuthResult::InvalidCredentials);
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
    test_authenticate_user_success();
    test_authenticate_user_invalid_credentials();
    test_send_chat_message_persists_and_notifies();
    test_fetch_chat_history_limit_and_is_mine();
    test_fetch_chat_history_caps_limit();
    return EXIT_SUCCESS;
}
