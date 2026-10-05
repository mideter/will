#pragma once

#include "matter/dating.h"
#include "matter/abode.h"
#include "matter/supplication.h"
#include "matter/execution.h"
#include "matter/placement.h"
#include "matter/tie.h"
#include "places/obedience.h"
#include "places/tie.h"
#include "matter/word.h"
#include "relations/supplication.h"
#include "places/abode.h"
#include "words/letter.h"
#include "words/recollection.h"
#include "relations/contemplation.h"
#include "men/soul.h"
#include "acts/creation.h"
#include "horizons/life.h"
#include "horizons/space.h"
#include "men/man.h"
#include "men/novice.h"
#include "men/testator.h"
#include "words/behest.h"
#include "men/vessel.h"
#include "horizons/world.h"
#include "identity/word.h"
#include "identity/place.h"
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
#include <memory>
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


/// The matter of the test dimensions, as a database file is for SQLite ones:
/// it outlives the dimensions that keep matter in it.
struct InMemoryShared {
	FakeTime time;
	std::uint64_t next_soul_id = 0;
	std::uint64_t next_vessel_id = 0;
	std::uint64_t next_word_id = 0;

	// Eternity
	std::vector<std::pair<id::Soul, SoulName>> souls;
	std::vector<matter::Word> words;

	// Spatiality
	std::vector<std::pair<id::Soul, matter::Abode>> abodes;
	std::vector<matter::Tie> ties;
	std::vector<matter::Placement> placements;

	// Temporality
	std::vector<matter::Vessel> vessels;
	std::vector<matter::Embodiment> embodiments;
	std::vector<matter::Dating> datings;
	std::vector<matter::Supplication> supplications;
	std::vector<matter::Execution> executions;
};


class InMemorySpatiality;
class InMemoryTemporality;


class InMemoryEternity final : public Eternity {
public:
	explicit InMemoryEternity(InMemoryShared& shared)
		: shared_(shared)
	{}

	Time& time() override { return shared_.time; }

	Space& space() override { return space_; }

	std::unique_ptr<Spatiality> spatiality(Birth<Creation>) override;

	std::unique_ptr<Temporality> temporality(Birth<Creation>) override;

	/// The dimensions realised for the present Creation; throw if none.
	InMemorySpatiality& realised_spatiality() const;
	InMemoryTemporality& realised_temporality() const;

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
		shared_.words.push_back(row);
		return row;
	}

	matter::Word word(const id::Word id) const override
	{
		for (const matter::Word& row : shared_.words) {
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
	InMemorySpatiality* spatiality_ = nullptr;
	InMemoryTemporality* temporality_ = nullptr;
};


class InMemorySpatiality final : public Spatiality {
public:
	InMemorySpatiality(InMemoryShared& shared, Eternity& eternity)
		: shared_(shared)
		, eternity_(eternity)
	{}

	std::optional<matter::Abode> abode(const id::Soul host) const override
	{
		for (const auto& [keeper, kept] : shared_.abodes) {
			if (keeper == host)
				return kept;
		}
		return std::nullopt;
	}

	matter::Abode abide(const id::Soul host, AbodeName name) override
	{
		if (abode(host))
			throw std::logic_error("soul already keeps an abode");

		const matter::Abode kept{eternity_.space().point(), std::move(name)};
		shared_.abodes.emplace_back(host, kept);
		return kept;
	}

	matter::Tie bind(const id::Soul testator, const id::Soul novice) override
	{
		for (const matter::Tie& row : shared_.ties) {
			if (row.testator() == testator && row.novice() == novice)
				throw std::logic_error("obedience already exists for this pair");
		}

		const matter::Tie kept{eternity_.space().point(), testator, novice};
		shared_.ties.push_back(kept);
		return kept;
	}

	std::vector<matter::Tie> ties() const override { return shared_.ties; }

	matter::Placement place(const id::Word id, const id::Place place) override
	{
		const matter::Placement placed{id, place};
		for (auto& row : shared_.placements) {
			if (row.id() == id) {
				row = placed;
				return placed;
			}
		}
		shared_.placements.push_back(placed);
		return placed;
	}

	std::optional<matter::Placement> placement(const id::Word id) const override
	{
		for (const matter::Placement& row : shared_.placements) {
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
		for (const matter::Placement& row : shared_.placements) {
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
	mutable std::size_t placement_reads_ = 0;
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
		shared_.vessels.emplace_back(vessel, std::move(token));
		shared_.embodiments.emplace_back(soul, vessel);
		return shared_.embodiments.back();
	}

	std::vector<matter::Vessel> vessels() const override { return shared_.vessels; }

	std::vector<matter::Embodiment> embodiments() const override { return shared_.embodiments; }

	matter::Dating date(const id::Word id) override
	{
		const matter::Dating dated{id, eternity_.time().instant()};
		for (auto& row : shared_.datings) {
			if (row.id() == id) {
				row = dated;
				return dated;
			}
		}
		shared_.datings.push_back(dated);
		return dated;
	}

	std::vector<matter::Dating> datings(const std::vector<id::Word>& ids) const override
	{
		std::vector<matter::Dating> out;
		for (const id::Word id : ids) {
			for (const matter::Dating& row : shared_.datings) {
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
		if (awaiting(suppliant, addressee) != shared_.supplications.end())
			throw std::logic_error("pending supplication already exists for this pair");

		shared_.supplications.push_back(kept);
		return kept;
	}

	std::vector<matter::Supplication> supplications(const id::Soul addressee) const override
	{
		std::vector<matter::Supplication> out;
		for (const matter::Supplication& row : shared_.supplications) {
			if (row.addressee() == addressee)
				out.push_back(row);
		}
		return out;
	}

	void reject(const id::Soul suppliant, const id::Soul addressee) override
	{
		const auto it = awaiting(suppliant, addressee);
		if (it == shared_.supplications.end())
			throw std::invalid_argument("unknown supplication");

		shared_.supplications.erase(it);
	}

	matter::Execution execute(const id::Word deed, const id::Word behest) override
	{
		for (const matter::Execution& row : shared_.executions) {
			if (row.behest() == behest)
				throw std::logic_error("behest is already executed");
		}
		shared_.executions.emplace_back(deed, behest);
		return shared_.executions.back();
	}

	std::vector<matter::Execution> executions(const std::vector<id::Word>& words) const override
	{
		std::vector<matter::Execution> out;
		for (const matter::Execution& row : shared_.executions) {
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
		for (auto it = shared_.supplications.begin(); it != shared_.supplications.end(); ++it) {
			if (it->suppliant() == suppliant && it->addressee() == addressee)
				return it;
		}
		return shared_.supplications.end();
	}

	InMemoryShared& shared_;
	Eternity& eternity_;
};


inline std::unique_ptr<Spatiality> InMemoryEternity::spatiality(Birth<Creation>)
{
	auto realised = std::make_unique<InMemorySpatiality>(shared_, *this);
	spatiality_ = realised.get();
	return realised;
}


inline std::unique_ptr<Temporality> InMemoryEternity::temporality(Birth<Creation>)
{
	auto realised = std::make_unique<InMemoryTemporality>(shared_, *this);
	temporality_ = realised.get();
	return realised;
}


inline InMemorySpatiality& InMemoryEternity::realised_spatiality() const
{
	if (spatiality_ == nullptr)
		throw std::logic_error("no Spatiality realised");
	return *spatiality_;
}


inline InMemoryTemporality& InMemoryEternity::realised_temporality() const
{
	if (temporality_ == nullptr)
		throw std::logic_error("no Temporality realised");
	return *temporality_;
}


/// In-memory Eternity and Life for tests; Creation realises the dimensions over
/// the shared matter. spatiality() / temporality() are those of Life's Creation.
/// A cosmos may keep its matter in a store that outlives it, as a server run
/// keeps it in files.
class InMemoryCosmos {
public:
	InMemoryCosmos()
		: InMemoryCosmos(own_)
	{}

	explicit InMemoryCosmos(InMemoryShared& shared)
		: shared_(shared)
		, eternity_(shared_)
	{}

	Life& life() { return life_; }
	InMemoryEternity& eternity() { return eternity_; }
	InMemorySpatiality& spatiality() { return eternity_.realised_spatiality(); }
	InMemoryTemporality& temporality() { return eternity_.realised_temporality(); }
	InMemoryShared& shared() { return shared_; }

	FakeTime& fake_time() { return shared_.time; }

private:
	InMemoryShared own_;
	InMemoryShared& shared_;
	InMemoryEternity eternity_;
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


/// Keep a soul and its vessel before Creation, as if kept by an earlier one.
inline void seed_man(InMemoryCosmos& cosmos, const id::Soul soul_id, const DeviceToken& token, const SoulName name)
{
	InMemoryShared& shared = cosmos.shared();
	shared.souls.emplace_back(soul_id, name);
	const id::Vessel vessel_id{soul_id.value()};
	shared.vessels.push_back(matter::Vessel{vessel_id, token});
	shared.embodiments.emplace_back(soul_id, vessel_id);
	if (soul_id.value() > shared.next_soul_id)
		shared.next_soul_id = soul_id.value();
	if (vessel_id.value() > shared.next_vessel_id)
		shared.next_vessel_id = vessel_id.value();
}


} // namespace will::domain::test
