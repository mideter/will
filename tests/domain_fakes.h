#pragma once

#include "values/device_token.h"
#include "entities/letter.h"
#include "entities/user.h"
#include "ids/user_id.h"
#include "values/user_name.h"
#include "ports/letter_repository.h"
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


class InMemoryLetterRepository final : public LetterRepository {
public:
    Letter append(AbodeId abode, UserId author, std::string_view body, Timestamp ts) override
    {
        Letter letter{LetterId{++next_id_}, abode, author, std::string(body), ts};
        letters_.push_back(letter);
        return letter;
    }

    std::vector<Letter> load_last(AbodeId abode, std::uint32_t limit) override
    {
        std::vector<Letter> matching;
        matching.reserve(letters_.size());
        for (const Letter& letter : letters_) {
            if (letter.abode_id() == abode)
                matching.push_back(letter);
        }
        if (limit >= matching.size())
            return matching;
        return std::vector<Letter>(matching.end() - static_cast<std::ptrdiff_t>(limit), matching.end());
    }

private:
    std::uint64_t next_id_ = 0;
    std::vector<Letter> letters_;
};


class FakeParticipantNotifier final : public ParticipantNotifier {
public:
    void notify_letter(const Letter& letter) override { notifications_.push_back(letter); }

    std::vector<Letter> notifications_;
};


} // namespace will::domain::test
