#pragma once

#include "values/device_token.h"
#include "entities/god.h"
#include "entities/letter.h"
#include "ids/god_id.h"
#include "values/god_name.h"
#include "ports/heaven.h"
#include "ports/letter_repository.h"
#include "ports/participant_notifier.h"

#include <map>
#include <optional>
#include <string>
#include <vector>


namespace will::domain::test {


class FakeHeaven final : public Heaven {
public:
    std::optional<God> find_by_device_token(const std::string_view device_token) override
    {
        const auto it = by_token_.find(std::string(device_token));
        if (it == by_token_.end())
            return std::nullopt;
        return gods_.at(it->second);
    }

    std::optional<God> find_by_id(const GodId id) override
    {
        const auto it = gods_.find(id);
        if (it == gods_.end())
            return std::nullopt;
        return it->second;
    }

    God create_god(const std::string_view device_token, const GodName name) override
    {
        const GodId id{++next_god_id_};
        God god{id, *DeviceToken::parse(device_token), name};
        gods_.emplace(id, god);
        by_token_.insert_or_assign(std::string(god.device_token().text()), id);
        return god;
    }

    void add_god(God god)
    {
        gods_.emplace(god.id(), god);
        by_token_.insert_or_assign(std::string(god.device_token().text()), god.id());
    }

private:
    std::uint64_t next_god_id_ = 0;
    std::map<GodId, God> gods_;
    std::map<std::string, GodId> by_token_;
};


class InMemoryLetterRepository final : public LetterRepository {
public:
    Letter append(AbodeId abode, GodId author, std::string_view body, Timestamp ts) override
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
