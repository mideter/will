#pragma once

#include "acts/dating.h"
#include "acts/abiding.h"
#include "acts/asking.h"
#include "acts/execution.h"
#include "acts/placement.h"
#include "acts/boundness.h"
#include "beings/immanents/obedience.h"
#include "beings/immanents/tie.h"
#include "acts/utterance.h"
#include "acts/supplication.h"
#include "beings/immanents/abode.h"
#include "beings/immanents/soul.h"
#include "beings/space.h"
#include "beings/immanents/man.h"
#include "beings/immanents/novice.h"
#include "beings/immanents/testator.h"
#include "beings/deed.h"
#include "beings/immanents/vessel.h"
#include "beings/world.h"
#include "identity/abode.h"
#include "identity/word.h"
#include "identity/place.h"
#include "identity/tie.h"
#include "identity/soul.h"
#include "identity/vessel.h"
#include "ports/eternity.h"
#include "ports/spatiality.h"
#include "ports/temporality.h"
#include "ports/time.h"
#include "values/abode_name.h"
#include "values/device_token.h"
#include "values/soul_name.h"
#include "values/timestamp.h"
#include "values/saying.h"

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

	id::Soul enroll(const SoulName name) override
	{
		const id::Soul soul_id{++shared_.next_soul_id};
		shared_.souls.emplace_back(soul_id, name);
		return soul_id;
	}

	Utterance utter(const id::Soul author, const Saying& saying) override
	{
		Utterance row{id::Word{++shared_.next_word_id}, author, saying};
		utterances_.push_back(row);
		return row;
	}

	Utterance utterance(const id::Word id) const override
	{
		for (const Utterance& row : utterances_) {
			if (row.id() == id)
				return row;
		}
		throw std::invalid_argument("unknown utterance");
	}

	std::vector<Utterance> utterances(const std::vector<id::Word>& ids) const override
	{
		std::vector<Utterance> out;
		out.reserve(ids.size());
		for (const id::Word id : ids) {
			try {
				out.push_back(utterance(id));
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
	std::vector<Utterance> utterances_;
};


class InMemorySpatiality final : public Spatiality {
public:
	InMemorySpatiality(InMemoryShared& shared, Eternity& eternity)
		: shared_(shared)
		, eternity_(eternity)
	{}

	std::vector<Abiding> abodes() override
	{
		std::vector<Abiding> out;
		out.reserve(abode_rows_.size());
		for (const auto& [id, name] : abode_rows_)
			out.emplace_back(id, name);
		return out;
	}

	Abiding abide(const id::Soul soul, AbodeName name) override
	{
		if (std::optional<Abiding> existing = abode_of(soul))
			return std::move(*existing);

		const id::Abode id{eternity_.space().point()};
		keep(id, name);
		join_abode(id, soul);
		return Abiding{id, std::move(name)};
	}

	std::optional<Abiding> abode_of(const id::Soul soul) const override
	{
		for (const auto& [abode_id, soul_id] : abode_souls_) {
			if (soul_id != soul)
				continue;
			for (const auto& [id, name] : abode_rows_) {
				if (id == abode_id)
					return Abiding{id, name};
			}
		}
		return std::nullopt;
	}

	void keep(const id::Abode id, AbodeName name) override
	{
		for (auto& row : abode_rows_) {
			if (row.first == id)
				return;
		}
		abode_rows_.emplace_back(id, std::move(name));
	}

	void join_abode(const id::Abode abode, const id::Soul soul) override
	{
		for (const auto& row : abode_souls_) {
			if (row.first == abode && row.second == soul)
				return;
		}
		abode_souls_.emplace_back(abode, soul);
	}

	void place(const id::Word id, const id::Place place) override
	{
		for (auto& row : placements_) {
			if (row.id() == id) {
				row = Placement{id, place};
				return;
			}
		}
		placements_.emplace_back(id, place);
	}

	std::optional<Placement> placement(const id::Word id) const override
	{
		for (const Placement& row : placements_) {
			if (row.id() == id)
				return row;
		}
		return std::nullopt;
	}

	std::vector<Placement> placements(const id::Place place, const std::uint32_t limit) const override
	{
		std::vector<Placement> matching;
		for (const Placement& row : placements_) {
			if (row.place() == place)
				matching.push_back(row);
		}
		if (limit >= matching.size())
			return matching;
		return std::vector<Placement>(matching.end() - static_cast<std::ptrdiff_t>(limit), matching.end());
	}

private:
	InMemoryShared& shared_;
	Eternity& eternity_;
	std::vector<std::pair<id::Abode, AbodeName>> abode_rows_;
	std::vector<std::pair<id::Abode, id::Soul>> abode_souls_;
	std::vector<Placement> placements_;
};


class InMemoryTemporality final : public Temporality {
public:
	InMemoryTemporality(InMemoryShared& shared, Eternity& eternity)
		: shared_(shared)
		, eternity_(eternity)
	{}

	Embodiment embody(const id::Soul soul, SoulName name, DeviceToken token) override
	{
		const id::Vessel vessel_id{++shared_.next_vessel_id};
		Embodiment row{soul, std::move(name), vessel_id, std::move(token)};
		embodiments_.push_back(row);
		return row;
	}

	std::vector<Embodiment> embodiments() const override { return embodiments_; }

	/// Keep soul + embodiment before the living World wakes (Creation load).
	void seed_man(const id::Soul soul_id, const DeviceToken& token, const SoulName name)
	{
		shared_.souls.emplace_back(soul_id, name);
		const id::Vessel vessel_id{soul_id.value()};
		embodiments_.push_back(Embodiment{soul_id, name, vessel_id, token});
		if (soul_id.value() > shared_.next_soul_id)
			shared_.next_soul_id = soul_id.value();
		if (vessel_id.value() > shared_.next_vessel_id)
			shared_.next_vessel_id = vessel_id.value();
	}

	Dating date(const id::Word id) override
	{
		const Dating dated{id, eternity_.time().instant()};
		for (auto& row : datings_) {
			if (row.id() == id) {
				row = dated;
				return dated;
			}
		}
		datings_.push_back(dated);
		return dated;
	}

	std::vector<Dating> datings(const std::vector<id::Word>& ids) const override
	{
		std::vector<Dating> out;
		for (const id::Word id : ids) {
			for (const Dating& row : datings_) {
				if (row.id() == id) {
					out.push_back(row);
					break;
				}
			}
		}
		return out;
	}

	Asking ask(const id::Soul suppliant, const id::Soul addressee) override
	{
		const Asking asking{suppliant, addressee};
		if (pair_exists(addressee, suppliant))
			throw std::logic_error("obedience already exists for this pair");
		if (awaiting(suppliant, addressee) != askings_.end())
			throw std::logic_error("pending supplication already exists for this pair");

		askings_.push_back(asking);
		return asking;
	}

	std::vector<Asking> askings(const id::Soul addressee) const override
	{
		std::vector<Asking> out;
		for (const Asking& row : askings_) {
			if (row.addressee() == addressee && !pair_exists(row.addressee(), row.suppliant()))
				out.push_back(row);
		}
		return out;
	}

	void reject(const id::Soul suppliant, const id::Soul addressee) override
	{
		const auto it = awaiting(suppliant, addressee);
		if (it == askings_.end() || pair_exists(addressee, suppliant))
			throw std::invalid_argument("unknown supplication");

		askings_.erase(it);
	}

	Boundness tie(const id::Soul testator, const id::Soul novice) override
	{
		if (pair_exists(testator, novice))
			throw std::logic_error("obedience already exists for this pair");

		const Boundness boundness{id::Tie{eternity_.space().point()}, testator, novice};
		tyings_.push_back(boundness);
		return boundness;
	}

	std::vector<Boundness> tyings() const override { return tyings_; }

	void execute(const id::Word deed) override
	{
		for (const Execution& row : executions_) {
			if (row.id() == deed)
				throw std::logic_error("deed is already executed");
		}
		executions_.emplace_back(deed, eternity_.time().instant());
	}

	std::vector<Execution> executions(const std::vector<id::Word>& ids) const override
	{
		std::vector<Execution> out;
		for (const id::Word id : ids) {
			for (const Execution& row : executions_) {
				if (row.id() == id) {
					out.push_back(row);
					break;
				}
			}
		}
		return out;
	}

private:
	bool pair_exists(const id::Soul testator, const id::Soul novice) const
	{
		for (const Boundness& row : tyings_) {
			if (row.testator() == testator && row.novice() == novice)
				return true;
		}
		return false;
	}

	/// Askings whose rejection was kept are dropped; the rest await or are tied.
	std::vector<Asking>::const_iterator awaiting(const id::Soul suppliant, const id::Soul addressee) const
	{
		for (auto it = askings_.begin(); it != askings_.end(); ++it) {
			if (it->suppliant() == suppliant && it->addressee() == addressee)
				return it;
		}
		return askings_.end();
	}

	InMemoryShared& shared_;
	Eternity& eternity_;
	std::vector<Embodiment> embodiments_;
	std::vector<Dating> datings_;
	std::vector<Asking> askings_;
	std::vector<Boundness> tyings_;
	std::vector<Execution> executions_;
};


/// In-memory three faces for tests; wire Creation with eternity/spatiality/temporality.
class InMemoryCosmos {
public:
	InMemoryCosmos()
		: eternity_(shared_)
		, spatiality_(shared_, eternity_)
		, temporality_(shared_, eternity_)
	{}

	InMemoryEternity& eternity() { return eternity_; }
	InMemorySpatiality& spatiality() { return spatiality_; }
	InMemoryTemporality& temporality() { return temporality_; }

	FakeTime& fake_time() { return shared_.time; }

private:
	InMemoryShared shared_;
	InMemoryEternity eternity_;
	InMemorySpatiality spatiality_;
	InMemoryTemporality temporality_;
};


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
