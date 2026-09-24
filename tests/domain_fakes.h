#pragma once

#include "acts/dating.h"
#include "acts/placement.h"
#include "acts/tying.h"
#include "acts/obedience.h"
#include "acts/tie.h"
#include "acts/utterance.h"
#include "acts/supplication.h"
#include "beings/abode.h"
#include "beings/soul.h"
#include "beings/man.h"
#include "beings/novice.h"
#include "beings/testator.h"
#include "beings/deed.h"
#include "beings/vessel.h"
#include "beings/world.h"
#include "identity/abode.h"
#include "identity/deed.h"
#include "identity/letter.h"
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
#include "values/word.h"

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
	std::uint64_t next_place_id = 0;
	std::uint64_t next_letter_id = 0;
	std::uint64_t next_deed_id = 0;
	std::vector<std::pair<id::Soul, SoulName>> souls;
};


class InMemoryEternity final : public Eternity {
public:
	explicit InMemoryEternity(InMemoryShared& shared)
		: shared_(shared)
	{}

	Time& time() override { return shared_.time; }

	id::Soul enroll(const SoulName name) override
	{
		const id::Soul soul_id{++shared_.next_soul_id};
		shared_.souls.emplace_back(soul_id, name);
		note_place(soul_id.value());
		return soul_id;
	}

	Utterance utter(const id::Soul author, const Word& word) override
	{
		Utterance row{id::Letter{++shared_.next_letter_id}, author, word};
		utterances_.push_back(row);
		return row;
	}

	Utterance utterance(const id::Letter id) const override
	{
		for (const Utterance& row : utterances_) {
			if (row.id() == id)
				return row;
		}
		throw std::invalid_argument("unknown utterance");
	}

	std::vector<Utterance> utterances(const std::vector<id::Letter>& ids) const override
	{
		std::vector<Utterance> out;
		out.reserve(ids.size());
		for (const id::Letter id : ids) {
			try {
				out.push_back(utterance(id));
			} catch (const std::invalid_argument&) {
			}
		}
		return out;
	}

	void remember(Utterance row) { utterances_.push_back(std::move(row)); }

	FakeTime& fake_time() { return shared_.time; }

	InMemoryShared& shared() { return shared_; }

private:
	void note_place(const std::uint64_t value)
	{
		if (value > shared_.next_place_id)
			shared_.next_place_id = value;
	}

	InMemoryShared& shared_;
	std::vector<Utterance> utterances_;
};


class InMemorySpatiality final : public Spatiality {
public:
	explicit InMemorySpatiality(InMemoryShared& shared)
		: shared_(shared)
	{}

	std::vector<Abode> abodes() override
	{
		std::vector<Abode> out;
		out.reserve(abode_rows_.size());
		for (const auto& [id, name] : abode_rows_)
			out.emplace_back(id, name);
		return out;
	}

	void keep(const id::Abode id, AbodeName name) override
	{
		note_place(id.value());
		for (auto& row : abode_rows_) {
			if (row.first == id)
				return;
		}
		abode_rows_.emplace_back(id, std::move(name));
	}

	void join_abode(const id::Abode, const id::Soul) override {}

	void place(const id::Letter id, const id::Place place) override
	{
		for (auto& row : placements_) {
			if (row.id() == id) {
				row = Placement{id, place};
				return;
			}
		}
		placements_.emplace_back(id, place);
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
	void note_place(const std::uint64_t value)
	{
		if (value > shared_.next_place_id)
			shared_.next_place_id = value;
	}

	InMemoryShared& shared_;
	std::vector<std::pair<id::Abode, AbodeName>> abode_rows_;
	std::vector<Placement> placements_;
};


class InMemoryTemporality final : public Temporality {
public:
	explicit InMemoryTemporality(InMemoryShared& shared)
		: shared_(shared)
	{}

	Embodiment embody(const id::Soul soul, SoulName name, DeviceToken token) override
	{
		const id::Vessel vessel_id{++shared_.next_vessel_id};
		note_place(soul.value());
		Embodiment row{soul, std::move(name), vessel_id, std::move(token)};
		embodiments_.push_back(row);
		return row;
	}

	std::vector<Embodiment> embodiments() const override { return embodiments_; }

	/// Remember soul + embodiment before the living World wakes (Creation load).
	void seed_man(const id::Soul soul_id, const DeviceToken& token, const SoulName name)
	{
		shared_.souls.emplace_back(soul_id, name);
		const id::Vessel vessel_id{soul_id.value()};
		embodiments_.push_back(Embodiment{soul_id, name, vessel_id, token});
		if (soul_id.value() > shared_.next_soul_id)
			shared_.next_soul_id = soul_id.value();
		if (vessel_id.value() > shared_.next_vessel_id)
			shared_.next_vessel_id = vessel_id.value();
		note_place(soul_id.value());
	}

	void date(const id::Letter id, const Timestamp at) override
	{
		for (auto& row : datings_) {
			if (row.id() == id) {
				row = Dating{id, at};
				return;
			}
		}
		datings_.emplace_back(id, at);
	}

	std::vector<Dating> datings(const std::vector<id::Letter>& ids) const override
	{
		std::vector<Dating> out;
		for (const id::Letter id : ids) {
			for (const Dating& row : datings_) {
				if (row.id() == id) {
					out.push_back(row);
					break;
				}
			}
		}
		return out;
	}

	Supplication
	supplicate(const Novice& suppliant, const Testator& addressee) override
	{
		const id::Soul suppliant_id = suppliant.Soul::id();
		const id::Soul addressee_id = addressee.Soul::id();
		if (suppliant_id == addressee_id)
			throw std::invalid_argument("supplication requires distinct suppliant and addressee");
		if (pair_exists(addressee_id, suppliant_id))
			throw std::logic_error("obedience already exists for this pair");
		for (const auto& row : pending_supplications_) {
			if (row.suppliant().Soul::id() == suppliant_id && row.addressee().Soul::id() == addressee_id)
				throw std::logic_error("pending supplication already exists for this pair");
		}

		Supplication row{suppliant, addressee};
		pending_supplications_.push_back(row);
		return row;
	}

	std::vector<Supplication> pending_supplications(const id::Soul addressee) const override
	{
		std::vector<Supplication> out;
		for (const auto& row : pending_supplications_) {
			if (row.addressee().Soul::id() == addressee)
				out.push_back(row);
		}
		return out;
	}

	Tying accept(const Supplication& ask) override
	{
		const auto it = find_pending(ask);
		if (pair_exists(it->addressee().Soul::id(), it->suppliant().Soul::id()))
			throw std::logic_error("obedience already exists for this pair");

		const Testator& place_testator = it->addressee();
		const Novice& place_novice = it->suppliant();
		drop_pending(it);

		const id::Tie oid{allocate_place()};
		obediences_.push_back(ObedienceRow{oid, place_testator.Soul::id(), place_novice.Soul::id()});
		return Tying{oid, place_testator.Soul::id(), place_novice.Soul::id()};
	}

	void reject(const Supplication& ask) override
	{
		drop_pending(find_pending(ask));
	}

	Tying tying(const id::Tie id) const override
	{
		for (const auto& row : obediences_) {
			if (row.id != id)
				continue;
			return Tying{row.id, row.testator, row.novice};
		}
		throw std::invalid_argument("unknown obedience");
	}

	std::vector<Tying> tyings() const override
	{
		std::vector<Tying> out;
		out.reserve(obediences_.size());
		for (const auto& row : obediences_)
			out.push_back(Tying{row.id, row.testator, row.novice});
		return out;
	}

	Deed will(const Obedience& obedience, const Soul& testator, const Word& word) override
	{
		const id::Tie tid{obedience.id().value()};
		if (!has_obedience(tid))
			throw std::invalid_argument("unknown obedience");
		if (obedience.testator().Soul::id() != testator.id())
			throw std::logic_error("only the testator may will in this obedience");

		DeedRow row{id::Deed{++shared_.next_deed_id},
					tid,
					obedience.testator().Soul::id(),
					obedience.novice().Soul::id(),
					word.body(),
					shared_.time.instant(),
					std::nullopt,
					std::nullopt};
		deed_rows_.push_back(row);
		return make_deed(row);
	}

	Deed execute(const Deed& deed) override
	{
		for (auto& row : deed_rows_) {
			if (row.id.value() != deed.id().value())
				continue;
			if (row.executed_at || row.cancelled_at)
				throw std::logic_error("deed is not open");
			if (!has_obedience(row.obedience))
				throw std::invalid_argument("unknown obedience");
			row.executed_at = shared_.time.instant();
			return make_deed(row);
		}
		throw std::invalid_argument("unknown deed");
	}

	Deed deed(const id::Deed id) const override
	{
		for (const auto& row : deed_rows_) {
			if (row.id == id)
				return make_deed(row);
		}
		throw std::invalid_argument("unknown deed");
	}

	std::vector<Deed> deeds(const id::Tie tie) const override
	{
		std::vector<Deed> out;
		for (const auto& row : deed_rows_) {
			if (row.obedience == tie)
				out.push_back(make_deed(row));
		}
		return out;
	}

	FakeTime& fake_time() { return shared_.time; }

private:
	struct ObedienceRow {
		id::Tie id;
		id::Soul testator;
		id::Soul novice;
	};

	struct DeedRow {
		id::Deed id;
		id::Tie obedience;
		id::Soul testator;
		id::Soul novice;
		std::string body;
		Timestamp created_at;
		std::optional<Timestamp> executed_at;
		std::optional<Timestamp> cancelled_at;
	};

	void note_place(const std::uint64_t value)
	{
		if (value > shared_.next_place_id)
			shared_.next_place_id = value;
	}

	std::uint64_t allocate_place() { return ++shared_.next_place_id; }

	bool pair_exists(const id::Soul testator, const id::Soul novice) const
	{
		for (const auto& row : obediences_) {
			if (row.testator == testator && row.novice == novice)
				return true;
		}
		return false;
	}

	bool has_obedience(const id::Tie id) const
	{
		for (const auto& row : obediences_) {
			if (row.id == id)
				return true;
		}
		return false;
	}

	Deed make_deed(const DeedRow& row) const
	{
		const auto& place = static_cast<const Novice&>(Soul::of(row.novice)).obedience(row.obedience);
		return Deed{row.id, dynamic_cast<const Tie&>(place), Word{row.body}, row.created_at,
					row.executed_at, row.cancelled_at};
	}

	std::vector<Supplication>::const_iterator find_pending(const Supplication& ask) const
	{
		for (auto it = pending_supplications_.begin(); it != pending_supplications_.end(); ++it) {
			if (it->suppliant().Soul::id() == ask.suppliant().Soul::id()
				&& it->addressee().Soul::id() == ask.addressee().Soul::id())
				return it;
		}
		throw std::invalid_argument("unknown supplication");
	}

	void drop_pending(const std::vector<Supplication>::const_iterator drop)
	{
		std::vector<Supplication> kept;
		kept.reserve(pending_supplications_.size() - 1);
		for (auto it = pending_supplications_.begin(); it != pending_supplications_.end(); ++it) {
			if (it == drop)
				continue;
			kept.push_back(*it);
		}
		pending_supplications_.swap(kept);
	}

	InMemoryShared& shared_;
	std::vector<Embodiment> embodiments_;
	std::vector<Dating> datings_;
	std::vector<Supplication> pending_supplications_;
	std::vector<ObedienceRow> obediences_;
	std::vector<DeedRow> deed_rows_;
};


/// In-memory three faces for tests; wire Creation with eternity/spatiality/temporality.
class InMemoryCosmos {
public:
	InMemoryCosmos()
		: eternity_(shared_)
		, spatiality_(shared_)
		, temporality_(shared_)
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
