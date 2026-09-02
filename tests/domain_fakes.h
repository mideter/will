#pragma once

#include "entities/earth.h"
#include "entities/god.h"
#include "entities/heaven.h"
#include "entities/letter.h"
#include "entities/vessel.h"
#include "ids/god_id.h"
#include "ids/vessel_id.h"
#include "ports/eternity.h"
#include "ports/letter_repository.h"
#include "ports/participant_notifier.h"
#include "values/device_token.h"
#include "values/god_name.h"

#include <string>
#include <utility>
#include <vector>


namespace will::domain::test {


class InMemoryEternity final : public Eternity {
public:
    std::vector<God> load_gods() override { return gods_; }

    std::vector<Vessel> load_vessels() override { return vessels_; }

    std::pair<God, Vessel> insert_god_with_vessel(const std::string_view device_token,
                                                  const GodName name) override
    {
        const GodId god_id{++next_god_id_};
        const VesselId vessel_id{++next_vessel_id_};
        const DeviceToken token = *DeviceToken::parse(device_token);
        God god{god_id, name};
        Vessel vessel{vessel_id, token, god_id};
        gods_.push_back(god);
        vessels_.push_back(vessel);
        return {god, vessel};
    }

private:
    std::uint64_t next_god_id_ = 0;
    std::uint64_t next_vessel_id_ = 0;
    std::vector<God> gods_;
    std::vector<Vessel> vessels_;
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


inline God register_god_with_vessel(Heaven& heaven, Earth& earth, Eternity& eternity,
                                    const std::string_view device_token, const GodName name)
{
    auto [god, vessel] = eternity.insert_god_with_vessel(device_token, name);
    heaven.insert(god);
    earth.insert(std::move(vessel));
    return god;
}


inline void seed_god_with_token(Heaven& heaven, Earth& earth, const GodId god_id, const DeviceToken& token,
                                const GodName name)
{
    heaven.insert(God{god_id, name});
    earth.insert(Vessel{VesselId{god_id.value()}, token, god_id});
}


} // namespace will::domain::test
