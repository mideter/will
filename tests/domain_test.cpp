#include "domain_fakes.h"

#include "entities/chat_id.h"
#include "entities/participant_id.h"
#include "errors/domain_error.h"
#include "errors/otp_error.h"
#include "support/phone_number.h"
#include "usecases/fetch_chat_history.h"
#include "usecases/request_otp.h"
#include "usecases/send_chat_message.h"
#include "usecases/verify_otp.h"

#include <cassert>
#include <cstdlib>
#include <variant>


namespace {


using namespace will::domain;
using namespace will::domain::test;


RequestOtpConfig test_request_otp_config()
{
    RequestOtpConfig config;
    config.otp_ttl_sec = 300;
    config.otp_length = 6;
    config.otp_cooldown_sec = 60;
    config.max_requests_per_ip = 2;
    config.ip_rate_window_sec = 3600;
    config.hash_salt = "test-salt";
    config.dev_fixed_otp = "123456";
    return config;
}


VerifyOtpConfig test_verify_otp_config()
{
    VerifyOtpConfig config;
    config.max_verify_attempts = 3;
    config.hash_salt = "test-salt";
    return config;
}


void test_request_otp_success()
{
    FakeOtpStore otp_store;
    FakeSmsSender sms_sender;
    FakeOtpHasher hasher;

    RequestOtp request_otp(otp_store, sms_sender, hasher, test_request_otp_config());
    const auto result = request_otp.execute(RequestOtpInput{"+15551234567", "10.0.0.1", 1000});
    assert(std::holds_alternative<RequestOtpSuccess>(result));

    assert(sms_sender.sent_.size() == 1);
    assert(sms_sender.sent_[0].phone == "+15551234567");
    assert(sms_sender.sent_[0].code == "123456");

    const std::optional<OtpChallenge> challenge = otp_store.find_challenge("+15551234567");
    assert(challenge.has_value());
    assert(challenge->code_hash == hasher.hash("123456", "test-salt"));
    assert(challenge->expires_at_ms == 1000 + 300'000);
    assert(challenge->peer_ip == "10.0.0.1");
}


void test_request_otp_invalid_phone()
{
    FakeOtpStore otp_store;
    FakeSmsSender sms_sender;
    FakeOtpHasher hasher;

    RequestOtp request_otp(otp_store, sms_sender, hasher, test_request_otp_config());
    const auto result = request_otp.execute(RequestOtpInput{"not-a-phone", "10.0.0.1", 1000});
    assert(std::holds_alternative<OtpError>(result));
    assert(std::get<OtpError>(result) == OtpError::InvalidPhone);
    assert(sms_sender.sent_.empty());
}


void test_request_otp_rate_limited_by_cooldown()
{
    FakeOtpStore otp_store;
    FakeSmsSender sms_sender;
    FakeOtpHasher hasher;
    RequestOtpConfig config = test_request_otp_config();

    RequestOtp request_otp(otp_store, sms_sender, hasher, config);
    assert(std::holds_alternative<RequestOtpSuccess>(
        request_otp.execute(RequestOtpInput{"+15551234567", "10.0.0.1", 1000})));

    const auto second = request_otp.execute(RequestOtpInput{"+15551234567", "10.0.0.1", 2000});
    assert(std::holds_alternative<OtpError>(second));
    assert(std::get<OtpError>(second) == OtpError::RateLimited);
    assert(sms_sender.sent_.size() == 1);
}


void test_verify_otp_success_creates_user()
{
    FakeUserRepository users;
    FakeOtpStore otp_store;
    FakeAuthSessionStore sessions;
    FakeOtpHasher hasher;
    FakeSmsSender sms_sender;

    RequestOtp request_otp(otp_store, sms_sender, hasher, test_request_otp_config());
    assert(std::holds_alternative<RequestOtpSuccess>(
        request_otp.execute(RequestOtpInput{"+15551234567", "10.0.0.1", 1000})));

    VerifyOtp verify_otp(users, otp_store, sessions, hasher, test_verify_otp_config());
    const auto result = verify_otp.execute(
        VerifyOtpInput{*PhoneNumber::parse("+15551234567"), "123456", 1500});
    assert(std::holds_alternative<VerifyOtpSuccess>(result));

    const VerifyOtpSuccess& success = std::get<VerifyOtpSuccess>(result);
    assert(success.user_id.value > 0);
    assert(!success.token.empty());
    assert(!otp_store.find_challenge("+15551234567").has_value());

    const std::optional<User> user = users.find_by_phone("+15551234567");
    assert(user.has_value());
    assert(user->id == success.user_id);

    const std::optional<Account> resolved = sessions.resolve_token(success.token);
    assert(resolved.has_value());
    assert(resolved->user_id == success.user_id);
}


void test_verify_otp_existing_user()
{
    FakeUserRepository users;
    FakeOtpStore otp_store;
    FakeAuthSessionStore sessions;
    FakeOtpHasher hasher;
    FakeSmsSender sms_sender;
    users.add_user(User{UserId{42}, "+15551234567"});

    RequestOtp request_otp(otp_store, sms_sender, hasher, test_request_otp_config());
    assert(std::holds_alternative<RequestOtpSuccess>(
        request_otp.execute(RequestOtpInput{"+15551234567", "10.0.0.1", 1000})));

    VerifyOtp verify_otp(users, otp_store, sessions, hasher, test_verify_otp_config());
    const auto result = verify_otp.execute(
        VerifyOtpInput{*PhoneNumber::parse("+15551234567"), "123456", 1500});
    assert(std::holds_alternative<VerifyOtpSuccess>(result));
    assert(std::get<VerifyOtpSuccess>(result).user_id == UserId{42});
}


void test_verify_otp_invalid_and_expired()
{
    FakeUserRepository users;
    FakeOtpStore otp_store;
    FakeAuthSessionStore sessions;
    FakeOtpHasher hasher;
    FakeSmsSender sms_sender;

    RequestOtp request_otp(otp_store, sms_sender, hasher, test_request_otp_config());
    assert(std::holds_alternative<RequestOtpSuccess>(
        request_otp.execute(RequestOtpInput{"+15551234567", "10.0.0.1", 1000})));

    VerifyOtp verify_otp(users, otp_store, sessions, hasher, test_verify_otp_config());

    const auto wrong_code = verify_otp.execute(
        VerifyOtpInput{*PhoneNumber::parse("+15551234567"), "000000", 1500});
    assert(std::holds_alternative<OtpError>(wrong_code));
    assert(std::get<OtpError>(wrong_code) == OtpError::InvalidCode);

    const auto expired = verify_otp.execute(
        VerifyOtpInput{*PhoneNumber::parse("+15551234567"), "123456", 400'000});
    assert(std::holds_alternative<OtpError>(expired));
    assert(std::get<OtpError>(expired) == OtpError::Expired);
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
    test_request_otp_success();
    test_request_otp_invalid_phone();
    test_request_otp_rate_limited_by_cooldown();
    test_verify_otp_success_creates_user();
    test_verify_otp_existing_user();
    test_verify_otp_invalid_and_expired();
    test_send_chat_message_persists_and_notifies();
    test_fetch_chat_history_limit_and_is_mine();
    test_fetch_chat_history_caps_limit();
    return EXIT_SUCCESS;
}
