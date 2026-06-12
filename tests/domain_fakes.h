#pragma once

#include "entities/account.h"
#include "entities/auth_token.h"
#include "entities/chat_id.h"
#include "entities/message.h"
#include "entities/otp_challenge.h"
#include "entities/participant_id.h"
#include "entities/user.h"
#include "entities/user_id.h"
#include "events/outbound_event.h"
#include "ports/auth_session_store.h"
#include "ports/message_repository.h"
#include "ports/otp_hasher.h"
#include "ports/otp_store.h"
#include "ports/participant_notifier.h"
#include "ports/sms_sender.h"
#include "ports/user_repository.h"

#include <map>
#include <optional>
#include <string>
#include <vector>


namespace will::domain::test {


class FakeUserRepository final : public UserRepository {
public:
    std::optional<User> find_by_phone(const std::string_view phone) override
    {
        const auto it = by_phone_.find(std::string(phone));
        if (it == by_phone_.end())
            return std::nullopt;
        return users_.at(it->second);
    }

    User create_user(const std::string_view phone) override
    {
        const UserId id{++next_user_id_};
        User user{id, std::string(phone)};
        users_.emplace(id, user);
        by_phone_[user.phone] = id;
        return user;
    }

    void add_user(User user)
    {
        users_.emplace(user.id, user);
        by_phone_[user.phone] = user.id;
    }

private:
    std::uint64_t next_user_id_ = 0;
    std::map<UserId, User> users_;
    std::map<std::string, UserId> by_phone_;
};


class FakeOtpStore final : public OtpStore {
public:
    void save_challenge(OtpChallenge challenge) override { challenges_[challenge.phone] = std::move(challenge); }

    std::optional<OtpChallenge> find_challenge(const std::string_view phone) override
    {
        const auto it = challenges_.find(std::string(phone));
        if (it == challenges_.end())
            return std::nullopt;
        return it->second;
    }

    void increment_attempts(const std::string_view phone) override
    {
        const auto it = challenges_.find(std::string(phone));
        if (it != challenges_.end())
            ++it->second.attempts;
    }

    void invalidate(const std::string_view phone) override { challenges_.erase(std::string(phone)); }

    std::size_t count_challenges_by_peer_ip(const std::string_view peer_ip, const TimestampMs since_ms) override
    {
        std::size_t count = 0;
        for (const auto& [phone, challenge] : challenges_) {
            (void)phone;
            if (challenge.peer_ip == peer_ip && challenge.expires_at_ms >= since_ms)
                ++count;
        }
        return count;
    }

private:
    std::map<std::string, OtpChallenge> challenges_;
};


class FakeSmsSender final : public SmsSender {
public:
    void send_otp(const PhoneNumber& phone, const std::string_view code) override
    {
        sent_.push_back(Sent{phone.e164(), std::string(code)});
    }

    struct Sent {
        std::string phone;
        std::string code;
    };

    std::vector<Sent> sent_;
};


class FakeOtpHasher final : public OtpHasher {
public:
    std::string hash(const std::string_view code, const std::string_view salt) const override
    {
        return std::string(salt) + ":" + std::string(code);
    }
};


class FakeAuthSessionStore final : public AuthSessionStore {
public:
    AuthToken issue_session(UserId user) override
    {
        const AuthToken token{"token-" + std::to_string(++next_token_)};
        sessions_[token.value] = Account{user, token, 0};
        return token;
    }

    std::optional<Account> resolve_token(AuthToken token) override
    {
        const auto it = sessions_.find(token.value);
        if (it == sessions_.end())
            return std::nullopt;
        return it->second;
    }

    void revoke(AuthToken token) override { sessions_.erase(token.value); }

private:
    std::uint64_t next_token_ = 0;
    std::map<std::string, Account> sessions_;
};


class InMemoryMessageRepository final : public MessageRepository {
public:
    Message append(ChatId chat, UserId author, std::string_view body, TimestampMs ts) override
    {
        Message msg;
        msg.id = ++next_id_;
        msg.chat_id = chat;
        msg.author_id = author;
        msg.body = std::string(body);
        msg.created_at = ts;
        messages_.push_back(msg);
        return msg;
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


} // namespace will::domain::test
