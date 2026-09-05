#pragma once

#include "entities/earth.h"
#include "entities/soul.h"
#include "entities/heaven.h"
#include "entities/letter.h"
#include "entities/man.h"
#include "entities/vessel.h"
#include "entities/world.h"
#include "identity/man.h"
#include "identity/soul.h"
#include "identity/vessel.h"
#include "ports/eternity.h"
#include "ports/letter_repository.h"
#include "ports/participant_notifier.h"
#include "values/device_token.h"
#include "values/soul_name.h"

#include <string>
#include <utility>
#include <vector>


namespace will::domain::test {


class InMemoryEternity final : public Eternity {
public:
	std::vector<Man> recall() override { return men_; }

	Man insert_man(const DeviceToken& token, const SoulName name) override
	{
		const id::Soul soul_id{++next_soul_id_};
		const id::Vessel vessel_id{++next_vessel_id_};
		const id::Man man_id{++next_man_id_};
		Man man{man_id, Soul{soul_id, name}, Vessel{vessel_id, token}};
		men_.push_back(man);
		return man;
	}

	/// Remember a man in eternity before the living World wakes (ctor load).
	void seed_man(const id::Soul soul_id, const DeviceToken& token, const SoulName name)
	{
		const id::Vessel vessel_id{soul_id.value()};
		const id::Man man_id{soul_id.value()};
		men_.push_back(Man{man_id, Soul{soul_id, name}, Vessel{vessel_id, token}});
		if (soul_id.value() > next_soul_id_)
			next_soul_id_ = soul_id.value();
		if (vessel_id.value() > next_vessel_id_)
			next_vessel_id_ = vessel_id.value();
		if (man_id.value() > next_man_id_)
			next_man_id_ = man_id.value();
	}

private:
	std::uint64_t next_soul_id_ = 0;
	std::uint64_t next_vessel_id_ = 0;
	std::uint64_t next_man_id_ = 0;
	std::vector<Man> men_;
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


inline Soul register_soul_with_vessel(World& world, const std::string_view device_token)
{
	const DeviceToken token = *DeviceToken::parse(device_token);
	const Man man = world.birth_man(token);
	return *world.find_by_id(man.soul_id());
}


inline void seed_man(InMemoryEternity& eternity, const id::Soul soul_id, const DeviceToken& token,
					 const SoulName name)
{
	eternity.seed_man(soul_id, token, name);
}


} // namespace will::domain::test
