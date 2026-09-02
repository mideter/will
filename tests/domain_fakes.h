#pragma once

#include "entities/letter.h"
#include "entities/god.h"
#include "ids/god_id.h"
#include "ports/eternity.h"
#include "ports/letter_repository.h"
#include "ports/participant_notifier.h"
#include "values/device_token.h"
#include "values/god_name.h"

#include <string>
#include <vector>


namespace will::domain::test {


class InMemoryEternity final : public Eternity {
public:
    std::vector<God> load_all() override { return gods_; }

    God insert(const std::string_view device_token, const GodName name) override
    {
        const GodId id{++next_god_id_};
        God god{id, *DeviceToken::parse(device_token), name};
        gods_.push_back(god);
        return god;
    }

private:
    std::uint64_t next_god_id_ = 0;
    std::vector<God> gods_;
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
