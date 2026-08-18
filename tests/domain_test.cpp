#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

import will.domain;


namespace {


using namespace will::domain;


class FakeUserRepository final : public UserRepository {
public:
    std::optional<User> find_by_device_token(const std::string_view device_token) override
    {
        const auto it = by_token_.find(std::string(device_token));
        if (it == by_token_.end())
            return std::nullopt;
        return users_.at(it->second);
    }

    User create_user(const std::string_view device_token, const std::string_view name) override
    {
        const UserId id{++next_user_id_};
        User user{id, std::string(device_token), std::string(name)};
        users_.emplace(id, user);
        by_token_[user.device_token] = id;
        return user;
    }

    void set_name(const UserId id, const std::string_view name) override
    {
        users_.at(id).name = std::string(name);
    }

    void add_user(User user)
    {
        users_.emplace(user.id, user);
        by_token_[user.device_token] = user.id;
    }

private:
    std::uint64_t next_user_id_ = 0;
    std::map<UserId, User> users_;
    std::map<std::string, UserId> by_token_;
};


class InMemoryMessageRepository final : public MessageRepository {
public:
    Message append(ChatId chat, UserId author, std::string_view body, Timestamp ts) override
    {
        messages_.push_back(Message{
            .id = ++next_id_,
            .chat_id = chat,
            .author_id = author,
            .body = std::string(body),
            .created_at = ts,
            .author_name = {},
        });
        return messages_.back();
    }

    std::vector<Message> load_last(ChatId chat, std::uint32_t limit) override
    {
        std::vector<Message> matching;
        matching.reserve(messages_.size());
        for (const Message& m : messages_) {
            if (m.chat_id == chat)
                matching.push_back(m);
        }
        if (limit >= matching.size())
            return matching;
        return std::vector<Message>(matching.end() - static_cast<std::ptrdiff_t>(limit), matching.end());
    }

private:
    std::uint64_t next_id_ = 0;
    std::vector<Message> messages_;
};


class FakeParticipantNotifier final : public ParticipantNotifier {
public:
    void notify_chat_message(ChatId chat, const Message& msg, ParticipantId except_participant) override
    {
        notifications_.push_back(Notification{chat, msg, except_participant});
    }

    void send_to_participant(ParticipantId id, const OutboundEvent& ev) override
    {
        direct_.push_back(Direct{id, ev});
    }

    struct Notification {
        ChatId chat;
        Message message;
        ParticipantId except;
    };

    struct Direct {
        ParticipantId participant;
        OutboundEvent event;
    };

    std::vector<Notification> notifications_;
    std::vector<Direct> direct_;
};


void test_authenticate_device_creates_user()
{
    FakeUserRepository users;
    AuthenticateDevice authenticate(users);

    const AuthToken token = DeviceToken::generate();
    const auto result = authenticate.execute(AuthenticateDeviceInput{token.value, *Timestamp::parse(1000)});
    assert(std::holds_alternative<AuthenticateDeviceSuccess>(result));

    const AuthenticateDeviceSuccess& success = std::get<AuthenticateDeviceSuccess>(result);
    assert(success.account.user_id.value > 0);
    assert(success.account.session_token == token);
    assert(UserName::is_valid(success.account.name));

    assert(users.find_by_device_token(token.value).has_value());
    assert(users.find_by_device_token(token.value)->id == success.account.user_id);
    assert(users.find_by_device_token(token.value)->name == success.account.name);
}


void test_authenticate_device_existing_user()
{
    FakeUserRepository users;
    users.add_user(User{UserId{42}, "abcd1234abcd1234abcd1234abcd1234", {}});

    AuthenticateDevice authenticate(users);
    const auto result = authenticate.execute(
        AuthenticateDeviceInput{"abcd1234abcd1234abcd1234abcd1234", *Timestamp::parse(1000)});
    assert(std::holds_alternative<AuthenticateDeviceSuccess>(result));
    assert(std::get<AuthenticateDeviceSuccess>(result).account.user_id == UserId{42});

    assert(users.find_by_device_token("abcd1234abcd1234abcd1234abcd1234").has_value());
    assert(UserName::is_valid(users.find_by_device_token("abcd1234abcd1234abcd1234abcd1234")->name));
}


void test_authenticate_device_keeps_existing_name()
{
    FakeUserRepository users;
    users.add_user(User{UserId{7}, "abcd1234abcd1234abcd1234abcd1234", "keptname"});

    AuthenticateDevice authenticate(users);
    const auto result = authenticate.execute(
        AuthenticateDeviceInput{"abcd1234abcd1234abcd1234abcd1234", *Timestamp::parse(1000)});
    assert(std::holds_alternative<AuthenticateDeviceSuccess>(result));

    assert(users.find_by_device_token("abcd1234abcd1234abcd1234abcd1234").has_value());
    assert(users.find_by_device_token("abcd1234abcd1234abcd1234abcd1234")->name == "keptname");
}


void test_authenticate_device_invalid_token()
{
    FakeUserRepository users;
    AuthenticateDevice authenticate(users);

    const auto result = authenticate.execute(AuthenticateDeviceInput{"short", *Timestamp::parse(1000)});
    assert(std::holds_alternative<AuthError>(result));
    assert(std::get<AuthError>(result) == AuthError::InvalidToken);
}


void test_send_chat_message_persists_and_notifies()
{
    InMemoryMessageRepository messages;
    FakeParticipantNotifier notifier;

    const Account account{UserId{7}, AuthToken{"tok"}, *Timestamp::parse(500), "sendname"};
    const ParticipantId sender{42};

    SendChatMessage send(messages, notifier);
    const Message saved =
        send.execute(SendChatMessageInput{account, sender, ChatId::global(), "hello", *Timestamp::parse(900)});

    assert(saved.id > 0);
    assert(saved.author_id == account.user_id);
    assert(saved.body == "hello");
    assert(saved.created_at == *Timestamp::parse(900));
    assert(saved.author_name == "sendname");

    const auto loaded = messages.load_last(ChatId::global(), 10);
    assert(loaded.size() == 1);
    assert(loaded[0].body == "hello");

    assert(notifier.notifications_.size() == 1);
    assert(notifier.notifications_[0].except == sender);
    assert(notifier.notifications_[0].message.id == saved.id);
    assert(notifier.notifications_[0].message.author_name == "sendname");
}


void test_fetch_chat_history_limit_and_is_mine()
{
    InMemoryMessageRepository messages;
    const ChatId chat = ChatId::global();
    const UserId me{10};
    const UserId other{20};

    messages.append(chat, other, "peer", *Timestamp::parse(1));
    messages.append(chat, me, "mine", *Timestamp::parse(2));
    messages.append(chat, other, "peer2", *Timestamp::parse(3));

    FetchChatHistory fetch(messages);

    const auto zero_limit = fetch.execute(FetchChatHistoryInput{Account{me, {}, {}}, chat, 0});
    assert(std::holds_alternative<DomainError>(zero_limit));
    assert(std::get<DomainError>(zero_limit).code == DomainErrorCode::InvalidArgument);

    const auto ok = fetch.execute(FetchChatHistoryInput{Account{me, {}, {}}, chat, 2});
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
        messages.append(chat, author, "m", *Timestamp::parse(i));

    FetchChatHistory fetch(messages);
    const auto ok = fetch.execute(
        FetchChatHistoryInput{Account{author, {}, {}}, chat, FetchChatHistory::MaxHistoryRequestLimit + 50});
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
