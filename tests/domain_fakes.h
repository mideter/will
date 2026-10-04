#pragma once

#include "matter/dating.h"
#include "matter/abode.h"
#include "matter/supplication.h"
#include "matter/execution.h"
#include "matter/placement.h"
#include "matter/tie.h"
#include "immanents/obedience.h"
#include "immanents/tie.h"
#include "matter/word.h"
#include "acts/supplication.h"
#include "immanents/abode.h"
#include "words/letter.h"
#include "immanents/contemplation.h"
#include "immanents/soul.h"
#include "horizons/life.h"
#include "horizons/space.h"
#include "immanents/man.h"
#include "immanents/novice.h"
#include "immanents/testator.h"
#include "words/behest.h"
#include "immanents/vessel.h"
#include "horizons/world.h"
#include "identity/abode.h"
#include "identity/word.h"
#include "identity/place.h"
#include "identity/tie.h"
#include "identity/soul.h"
#include "identity/vessel.h"
#include "dimensions/eternity.h"
#include "dimensions/spatiality.h"
#include "dimensions/temporality.h"
#include "horizons/time.h"
#include "values/abode_name.h"
#include "values/device_token.h"
#include "values/soul_name.h"
#include "values/timestamp.h"
#include "values/saying.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


namespace will::domain::test {


class FakeTime final : public Time {
public:
	explicit FakeTime(const Timestamp instant = Timestamp{1})
		: instant_(instant)
	{}

	Timestamp instant() const override { return instant_; }

	void set_instant(const Timestamp instant) { instant_ = instant; }

private:
	Timestamp instant_;
};


struct InMemoryShared {
	FakeTime time;
	std::uint64_t next_soul_id = 0;
	std::uint64_t next_vessel_id = 0;
	std::uint64_t next_word_id = 0;
	std::vector<std::pair<id::Soul, SoulName>> souls;
};


class InMemoryEternity final : public Eternity {
public:
	explicit InMemoryEternity(InMemoryShared& shared)
		: shared_(shared)
	{}

	Time& time() override { return shared_.time; }

	Space& space() override { return space_; }

	matter::Soul enroll(const SoulName name) override
	{
		const id::Soul soul_id{++shared_.next_soul_id};
		shared_.souls.emplace_back(soul_id, name);
		return matter::Soul{soul_id, name};
	}

	std::vector<matter::Soul> souls() const override
	{
		std::vector<matter::Soul> out;
		out.reserve(shared_.souls.size());
		for (const auto& [id, name] : shared_.souls)
			out.emplace_back(id, name);
		return out;
	}

	matter::Word utter(const id::Soul author, const Saying& saying) override
	{
		matter::Word row{id::Word{++shared_.next_word_id}, author, saying};
		words_.push_back(row);
		return row;
	}

	matter::Word word(const id::Word id) const override
	{
		for (const matter::Word& row : words_) {
			if (row.id() == id)
				return row;
		}
		throw std::invalid_argument("unknown word");
	}

	std::vector<matter::Word> words(const std::vector<id::Word>& ids) const override
	{
		std::vector<matter::Word> out;
		out.reserve(ids.size());
		for (const id::Word id : ids) {
			try {
				out.push_back(word(id));
			} catch (const std::invalid_argument&) {
			}
		}
		return out;
	}

	FakeTime& fake_time() { return shared_.time; }

	InMemoryShared& shared() { return shared_; }

private:
	InMemoryShared& shared_;
	Space space_;
	std::vector<matter::Word> words_;
};


class InMemorySpatiality final : public Spatiality {
public:
	InMemorySpatiality(InMemoryShared& shared, Eternity& eternity)
		: shared_(shared)
		, eternity_(eternity)
	{}

	std::optional<matter::Abode> abode(const id::Soul host) const override
	{
		for (const auto& [keeper, kept] : abodes_) {
			if (keeper == host)
				return kept;
		}
		return std::nullopt;
	}

	matter::Abode abide(const id::Soul host, AbodeName name) override
	{
		if (abode(host))
			throw std::logic_error("soul already keeps an abode");

		const matter::Abode kept{id::Abode{eternity_.space().point()}, std::move(name)};
		abodes_.emplace_back(host, kept);
		return kept;
	}

	matter::Tie bind(const id::Soul testator, const id::Soul novice) override
	{
		for (const matter::Tie& row : ties_) {
			if (row.testator() == testator && row.novice() == novice)
				throw std::logic_error("obedience already exists for this pair");
		}

		const matter::Tie kept{id::Tie{eternity_.space().point()}, testator, novice};
		ties_.push_back(kept);
		return kept;
	}

	std::vector<matter::Tie> ties() const override { return ties_; }

	matter::Placement place(const id::Word id, const id::Place place) override
	{
		const matter::Placement placed{id, place};
		for (auto& row : placements_) {
			if (row.id() == id) {
				row = placed;
				return placed;
			}
		}
		placements_.push_back(placed);
		return placed;
	}

	std::optional<matter::Placement> placement(const id::Word id) const override
	{
		for (const matter::Placement& row : placements_) {
			if (row.id() == id)
				return row;
		}
		return std::nullopt;
	}

	/// How many times the words of a place were read from this dimension.
	std::size_t placement_reads() const noexcept { return placement_reads_; }

	std::vector<matter::Placement> placements(const id::Place place, const std::uint32_t limit) const override
	{
		++placement_reads_;

		std::vector<matter::Placement> matching;
		for (const matter::Placement& row : placements_) {
			if (row.place() == place)
				matching.push_back(row);
		}
		if (limit >= matching.size())
			return matching;
		return std::vector<matter::Placement>(matching.end() - static_cast<std::ptrdiff_t>(limit), matching.end());
	}

private:
	InMemoryShared& shared_;
	Eternity& eternity_;
	std::vector<std::pair<id::Soul, matter::Abode>> abodes_;
	std::vector<matter::Placement> placements_;
	mutable std::size_t placement_reads_ = 0;
	std::vector<matter::Tie> ties_;
};


class InMemoryTemporality final : public Temporality {
public:
	InMemoryTemporality(InMemoryShared& shared, Eternity& eternity)
		: shared_(shared)
		, eternity_(eternity)
	{}

	matter::Embodiment embody(const id::Soul soul, DeviceToken token) override
	{
		const id::Vessel vessel{++shared_.next_vessel_id};
		vessels_.emplace_back(vessel, std::move(token));
		embodiments_.emplace_back(soul, vessel);
		return embodiments_.back();
	}

	std::vector<matter::Vessel> vessels() const override { return vessels_; }

	std::vector<matter::Embodiment> embodiments() const override { return embodiments_; }

	/// Keep a soul and its vessel before the living World wakes (Creation load).
	void seed_man(const id::Soul soul_id, const DeviceToken& token, const SoulName name)
	{
		shared_.souls.emplace_back(soul_id, name);
		const id::Vessel vessel_id{soul_id.value()};
		vessels_.push_back(matter::Vessel{vessel_id, token});
		embodiments_.emplace_back(soul_id, vessel_id);
		if (soul_id.value() > shared_.next_soul_id)
			shared_.next_soul_id = soul_id.value();
		if (vessel_id.value() > shared_.next_vessel_id)
			shared_.next_vessel_id = vessel_id.value();
	}

	matter::Dating date(const id::Word id) override
	{
		const matter::Dating dated{id, eternity_.time().instant()};
		for (auto& row : datings_) {
			if (row.id() == id) {
				row = dated;
				return dated;
			}
		}
		datings_.push_back(dated);
		return dated;
	}

	std::vector<matter::Dating> datings(const std::vector<id::Word>& ids) const override
	{
		std::vector<matter::Dating> out;
		for (const id::Word id : ids) {
			for (const matter::Dating& row : datings_) {
				if (row.id() == id) {
					out.push_back(row);
					break;
				}
			}
		}
		return out;
	}

	matter::Supplication ask(const id::Soul suppliant, const id::Soul addressee) override
	{
		const matter::Supplication kept{suppliant, addressee};
		if (awaiting(suppliant, addressee) != supplications_.end())
			throw std::logic_error("pending supplication already exists for this pair");

		supplications_.push_back(kept);
		return kept;
	}

	std::vector<matter::Supplication> supplications(const id::Soul addressee) const override
	{
		std::vector<matter::Supplication> out;
		for (const matter::Supplication& row : supplications_) {
			if (row.addressee() == addressee)
				out.push_back(row);
		}
		return out;
	}

	void reject(const id::Soul suppliant, const id::Soul addressee) override
	{
		const auto it = awaiting(suppliant, addressee);
		if (it == supplications_.end())
			throw std::invalid_argument("unknown supplication");

		supplications_.erase(it);
	}

	matter::Execution execute(const id::Word deed, const id::Word behest) override
	{
		for (const matter::Execution& row : executions_) {
			if (row.behest() == behest)
				throw std::logic_error("behest is already executed");
		}
		executions_.emplace_back(deed, behest);
		return executions_.back();
	}

	std::vector<matter::Execution> executions(const std::vector<id::Word>& words) const override
	{
		std::vector<matter::Execution> out;
		for (const matter::Execution& row : executions_) {
			for (const id::Word word : words) {
				if (row.id() == word || row.behest() == word) {
					out.push_back(row);
					break;
				}
			}
		}
		return out;
	}

private:
	/// Supplications whose rejection was kept are dropped.
	std::vector<matter::Supplication>::const_iterator awaiting(const id::Soul suppliant, const id::Soul addressee) const
	{
		for (auto it = supplications_.begin(); it != supplications_.end(); ++it) {
			if (it->suppliant() == suppliant && it->addressee() == addressee)
				return it;
		}
		return supplications_.end();
	}

	InMemoryShared& shared_;
	Eternity& eternity_;
	std::vector<matter::Vessel> vessels_;
	std::vector<matter::Embodiment> embodiments_;
	std::vector<matter::Dating> datings_;
	std::vector<matter::Supplication> supplications_;
	std::vector<matter::Execution> executions_;
};


/// In-memory three faces for tests; wire Creation with eternity/spatiality/temporality.
class InMemoryCosmos {
public:
	InMemoryCosmos()
		: eternity_(shared_)
		, spatiality_(shared_, eternity_)
		, temporality_(shared_, eternity_)
		, life_(spatiality_, temporality_)
	{}

	Life& life() { return life_; }
	InMemoryEternity& eternity() { return eternity_; }
	InMemorySpatiality& spatiality() { return spatiality_; }
	InMemoryTemporality& temporality() { return temporality_; }

	FakeTime& fake_time() { return shared_.time; }

private:
	InMemoryShared shared_;
	InMemoryEternity eternity_;
	InMemorySpatiality spatiality_;
	InMemoryTemporality temporality_;
	Life life_;
};


/// The letters a gaze beholds (the words of an abode are letters).
inline std::vector<std::shared_ptr<const Letter>> letters_seen_by(const std::shared_ptr<const Contemplation>& gaze)
{
	std::vector<std::shared_ptr<const Letter>> letters;
	for (const std::shared_ptr<const Word>& word : gaze->words()) {
		if (auto letter = std::dynamic_pointer_cast<const Letter>(word))
			letters.push_back(std::move(letter));
	}
	return letters;
}


inline const Soul& register_soul_with_vessel(World& world, const std::string_view device_token)
{
	const DeviceToken token = *DeviceToken::parse(device_token);
	return world.welcome(token);
}


inline void seed_man(InMemoryTemporality& temporality, const id::Soul soul_id, const DeviceToken& token,
					 const SoulName name)
{
	temporality.seed_man(soul_id, token, name);
}


} // namespace will::domain::test
