#pragma once

#include "entities/account.h"
#include "entities/auth_token.h"
#include "entities/chat_id.h"
#include "entities/message.h"
#include "entities/participant_id.h"
#include "entities/user.h"
#include "entities/user_id.h"
#include "events/outbound_event.h"
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
    std::optional<User> find_by_device_token(const std::string_view device_token) override
    {
        const auto it = by_token_.find(std::string(device_token));
        if (it == by_token_.end())
            return std::nullopt;
        return users_.at(it->second);
    }

    User create_user(const std::string_view device_token) override
    {
        const UserId id{++next_user_id_};
        User user{id, std::string(device_token)};
        users_.emplace(id, user);
        by_token_[user.device_token] = id;
        return user;
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
