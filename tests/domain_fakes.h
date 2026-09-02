#pragma once

#include "values/device_token.h"
#include "entities/message.h"
#include "entities/user.h"
#include "ids/user_id.h"
#include "values/user_name.h"
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

    std::optional<User> find_by_id(const UserId id) override
    {
        const auto it = users_.find(id);
        if (it == users_.end())
            return std::nullopt;
        return it->second;
    }

    User create_user(const std::string_view device_token, const UserName name) override
    {
        const UserId id{++next_user_id_};
        User user{id, *DeviceToken::parse(device_token), name};
        users_.emplace(id, user);
        by_token_.insert_or_assign(std::string(user.device_token().text()), id);
        return user;
    }

    void add_user(User user)
    {
        users_.emplace(user.id(), user);
        by_token_.insert_or_assign(std::string(user.device_token().text()), user.id());
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
        Message msg{MessageId{++next_id_}, chat, author, std::string(body), ts};
        messages_.push_back(msg);
        return msg;
    }

    std::vector<Message> load_last(ChatId chat, std::uint32_t limit) override
    {
        std::vector<Message> matching;
        matching.reserve(messages_.size());
        for (const Message& m : messages_) {
            if (m.chat_id() == chat)
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
    void notify_chat_message(const Message& msg) override { notifications_.push_back(msg); }

    std::vector<Message> notifications_;
};


} // namespace will::domain::test
