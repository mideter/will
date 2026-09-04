#pragma once

#include "entities/dead_vessel.h"
#include "entities/earth.h"
#include "entities/soul.h"
#include "entities/heaven.h"
#include "entities/letter.h"
#include "entities/man.h"
#include "entities/vessel.h"
#include "ids/soul.h"
#include "ids/vessel.h"
#include "ports/eternity.h"
#include "ports/letter_repository.h"
#include "ports/participant_notifier.h"
#include "values/soul_name.h"

#include <string>
#include <utility>
#include <vector>


namespace will::domain::test {


class InMemoryEternity final : public Eternity {
public:
    std::vector<Soul> load_souls() override { return souls_; }

    std::vector<Vessel> load_vessels() override { return vessels_; }

    Man insert_soul_with_vessel(const DeadVessel& dead, const SoulName name) override
    {
        const id::Soul soul_id{++next_soul_id_};
        const id::Vessel vessel_id{++next_vessel_id_};
        Soul soul{soul_id, name};
        Vessel vessel{vessel_id, dead, soul_id};
        souls_.push_back(soul);
        vessels_.push_back(vessel);
        return Man{std::move(soul), std::move(vessel)};
    }

private:
    std::uint64_t next_soul_id_ = 0;
    std::uint64_t next_vessel_id_ = 0;
    std::vector<Soul> souls_;
    std::vector<Vessel> vessels_;
};


class InMemoryLetterRepository final : public LetterRepository {
public:
    Letter append(id::Abode abode, id::Soul author, std::string_view body, Timestamp ts) override
    {
        Letter letter{id::Letter{++next_id_}, abode, author, std::string(body), ts};
        letters_.push_back(letter);
        return letter;
    }

    std::vector<Letter> load_last(id::Abode abode, std::uint32_t limit) override
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


inline Soul register_soul_with_vessel(Heaven& heaven, Earth& earth, const std::string_view device_token)
{
    const DeadVessel dead{device_token};
    Man man = heaven.remember_with_vessel(dead);
    earth.insert(man.vessel());
    return man.soul();
}


inline void seed_soul_with_dead(Heaven& heaven, Earth& earth, const id::Soul soul_id, const DeadVessel& dead,
                               const SoulName name)
{
    heaven.insert(Soul{soul_id, name});
    earth.insert(Vessel{id::Vessel{soul_id.value()}, dead, soul_id});
}


} // namespace will::domain::test
