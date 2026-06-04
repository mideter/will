#pragma once

#include "entities/account.h"
#include "entities/auth_token.h"
#include "entities/chat_id.h"
#include "entities/message.h"
#include "entities/participant_id.h"
#include "entities/user.h"
#include "entities/user_id.h"
#include "events/outbound_event.h"
#include "ports/auth_session_store.h"
#include "ports/message_repository.h"
#include "ports/participant_notifier.h"
#include "ports/user_repository.h"

#include <map>
#include <optional>
#include <string>
#include <vector>


namespace will::domain::test {


class FakeUserRepository final : public UserRepository {
public:
    void add_user(User user, std::string password)
    {
        const UserId id = user.id;
        const std::string login = user.login;
        users_.emplace(id, UserRecord{std::move(user), std::move(password)});
        by_login_[login] = id;
    }

    std::optional<User> find_by_login(std::string_view login) override
    {
        const auto it = by_login_.find(std::string(login));
        if (it == by_login_.end())
            return std::nullopt;
        return users_.at(it->second).user;
    }

    bool verify_password(UserId id, std::string_view password) override
    {
        const auto it = users_.find(id);
        if (it == users_.end())
            return false;
        return it->second.password == password;
    }

private:
    struct UserRecord {
        User user;
        std::string password;
    };

    std::map<UserId, UserRecord> users_;
    std::map<std::string, UserId> by_login_;
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
