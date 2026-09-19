#pragma once

#include "beings/abode.h"
#include "acts/obedience.h"
#include "beings/soul.h"
#include "beings/letter.h"
#include "beings/man.h"
#include "acts/supplication.h"
#include "beings/deed.h"
#include "beings/vessel.h"
#include "beings/world.h"
#include "identity/abode.h"
#include "identity/deed.h"
#include "identity/obedience.h"
#include "identity/soul.h"
#include "identity/supplication.h"
#include "identity/vessel.h"
#include "ports/temporality.h"
#include "ports/time.h"
#include "values/abode_name.h"
#include "values/device_token.h"
#include "values/soul_name.h"
#include "values/timestamp.h"
#include "values/word.h"

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


class InMemoryTemporality final : public Temporality {
public:
	Time& time() override { return time_; }

	id::Soul enroll(const SoulName name) override
	{
		const id::Soul soul_id{++next_soul_id_};
		souls_.emplace_back(soul_id, name);
		return soul_id;
	}

	Embodiment embody(const id::Soul soul, DeviceToken token) override
	{
		const SoulName* name = nullptr;
		for (const auto& row : souls_) {
			if (row.first == soul) {
				name = &row.second;
				break;
			}
		}
		if (!name)
			throw std::invalid_argument("unknown soul");

		const id::Vessel vessel_id{++next_vessel_id_};
		note_place(soul.value());
		Embodiment row{soul, *name, vessel_id, std::move(token)};
		embodiments_.push_back(row);
		return row;
	}

	std::vector<Embodiment> embodiments() const override { return embodiments_; }

	/// Remember soul + embodiment before the living World wakes (Creation load).
	void seed_man(const id::Soul soul_id, const DeviceToken& token, const SoulName name)
	{
		souls_.emplace_back(soul_id, name);
		const id::Vessel vessel_id{soul_id.value()};
		embodiments_.push_back(Embodiment{soul_id, name, vessel_id, token});
		if (soul_id.value() > next_soul_id_)
			next_soul_id_ = soul_id.value();
		if (vessel_id.value() > next_vessel_id_)
			next_vessel_id_ = vessel_id.value();
		note_place(soul_id.value());
	}

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

	void fix(id::Place place, id::Soul author, const Word& word) const override
	{
		letters_.push_back(Letter{id::Letter{++next_id_}, place, author, word, time_.instant()});
	}

	std::vector<Letter> letters(id::Place place, std::uint32_t limit) const override
	{
		std::vector<Letter> matching;
		matching.reserve(letters_.size());
		for (const Letter& letter : letters_) {
			if (letter.place_id() == place)
				matching.push_back(letter);
		}
		if (limit >= matching.size())
			return matching;
		return std::vector<Letter>(matching.end() - static_cast<std::ptrdiff_t>(limit), matching.end());
	}

	Supplication
	supplicate(const Soul& suppliant, const Soul& addressee) override
	{
		const id::Soul suppliant_id = suppliant.id();
		const id::Soul addressee_id = addressee.id();
		if (suppliant_id == addressee_id)
			throw std::invalid_argument("supplication requires distinct suppliant and addressee");
		if (living_pair(addressee_id, suppliant_id))
			throw std::logic_error("living obedience already exists for this pair");
		for (const auto& row : supplications_) {
			if (row.suppliant().id() == suppliant_id && row.addressee().id() == addressee_id
				&& row.status() == SupplicationStatus::pending)
				throw std::logic_error("pending supplication already exists for this pair");
		}

		Supplication row{id::Supplication{++next_supplication_id_}, suppliant, addressee,
						 SupplicationStatus::pending, time_.instant()};
		supplications_.push_back(row);
		return row;
	}

	std::vector<Supplication> pending_supplications(const id::Soul addressee) const override
	{
		std::vector<Supplication> out;
		for (const auto& row : supplications_) {
			if (row.addressee().id() == addressee && row.status() == SupplicationStatus::pending)
				out.push_back(row);
		}
		return out;
	}

	Obedience accept(const id::Supplication id) override
	{
		Supplication& row = mutable_supplication(id);
		if (row.status() != SupplicationStatus::pending)
			throw std::logic_error("supplication is not pending");
		if (living_pair(row.addressee().id(), row.suppliant().id()))
			throw std::logic_error("living obedience already exists for this pair");

		replace_status(row, SupplicationStatus::accepted);

		const id::Obedience oid{allocate_place()};
		obediences_.push_back(
			ObedienceRow{oid, row.addressee().id(), row.suppliant().id(), true});
		return Obedience{oid, row.addressee(), row.suppliant(), true};
	}

	void refuse(const id::Supplication id) override
	{
		Supplication& row = mutable_supplication(id);
		if (row.status() != SupplicationStatus::pending)
			throw std::logic_error("supplication is not pending");
		replace_status(row, SupplicationStatus::refused);
	}

	Obedience obedience(const id::Obedience id) const override
	{
		for (const auto& row : obediences_) {
			if (row.id != id)
				continue;

			return Obedience{row.id, Soul::of(row.testator), Soul::of(row.executor), row.living};
		}
		throw std::invalid_argument("unknown obedience");
	}

	std::vector<Obedience> obediences() const override
	{
		std::vector<Obedience> out;
		out.reserve(obediences_.size());
		for (const auto& row : obediences_) {
			out.push_back(
				Obedience{row.id, Soul::of(row.testator), Soul::of(row.executor), row.living});
		}
		return out;
	}

	void secede(const id::Obedience id) override
	{
		for (auto& row : obediences_) {
			if (row.id != id)
				continue;
			if (!row.living)
				throw std::logic_error("obedience is not living");
			row.living = false;
			const Timestamp now = time_.instant();
			for (auto& deed : deed_rows_) {
				if (deed.obedience == id && !deed.executed_at && !deed.cancelled_at)
					deed.cancelled_at = now;
			}
			return;
		}
		throw std::invalid_argument("unknown obedience");
	}

	Deed bequeath(const Obedience& obedience, const Soul& testator, const Word& word) override
	{
		if (!living_obedience(obedience.obedience_id()))
			throw std::logic_error("obedience is not living");
		if (obedience.testator().id() != testator.id())
			throw std::logic_error("only the testator may bequeath in this obedience");

		DeedRow row{id::Deed{++next_deed_id_},
					obedience.obedience_id(),
					obedience.testator().id(),
					obedience.executor().id(),
					word.body(),
					time_.instant(),
					std::nullopt,
					std::nullopt};
		deed_rows_.push_back(row);
		return Deed{row.id, this->obedience(obedience.obedience_id()), word, row.created_at};
	}

	Deed execute(const Deed& deed) override
	{
		for (auto& row : deed_rows_) {
			if (row.id.value() != deed.id().value())
				continue;
			if (row.executed_at || row.cancelled_at)
				throw std::logic_error("deed is not open");
			if (!living_obedience(row.obedience))
				throw std::logic_error("obedience is not living");
			row.executed_at = time_.instant();
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

	std::vector<Deed> deeds(const id::Obedience obedience) const override
	{
		std::vector<Deed> out;
		for (const auto& row : deed_rows_) {
			if (row.obedience == obedience)
				out.push_back(make_deed(row));
		}
		return out;
	}

	FakeTime& fake_time() { return time_; }

private:
	struct ObedienceRow {
		id::Obedience id;
		id::Soul testator;
		id::Soul executor;
		bool living;
	};

	struct DeedRow {
		id::Deed id;
		id::Obedience obedience;
		id::Soul testator;
		id::Soul executor;
		std::string body;
		Timestamp created_at;
		std::optional<Timestamp> executed_at;
		std::optional<Timestamp> cancelled_at;
	};

	void note_place(const std::uint64_t value)
	{
		if (value > next_place_id_)
			next_place_id_ = value;
	}

	std::uint64_t allocate_place() { return ++next_place_id_; }

	bool living_pair(const id::Soul testator, const id::Soul executor) const
	{
		for (const auto& row : obediences_) {
			if (row.living && row.testator == testator && row.executor == executor)
				return true;
		}
		return false;
	}

	bool living_obedience(const id::Obedience id) const
	{
		for (const auto& row : obediences_) {
			if (row.id == id)
				return row.living;
		}
		return false;
	}

	Deed make_deed(const DeedRow& row) const
	{
		const Obedience obedience = this->obedience(row.obedience);
		return Deed{row.id, obedience, Word{row.body}, row.created_at, row.executed_at, row.cancelled_at};
	}

	Supplication& mutable_supplication(const id::Supplication id)
	{
		for (auto& row : supplications_) {
			if (row.id() == id)
				return row;
		}
		throw std::invalid_argument("unknown supplication");
	}

	static void replace_status(Supplication& row, const SupplicationStatus status)
	{
		const id::Supplication id = row.id();
		const Soul& suppliant = row.suppliant();
		const Soul& addressee = row.addressee();
		Timestamp created_at = row.created_at();
		std::destroy_at(&row);
		std::construct_at(&row, id, suppliant, addressee, status, std::move(created_at));
	}

	FakeTime time_;
	std::uint64_t next_soul_id_ = 0;
	std::uint64_t next_vessel_id_ = 0;
	std::uint64_t next_place_id_ = 0;
	std::uint64_t next_supplication_id_ = 0;
	std::uint64_t next_deed_id_ = 0;
	mutable std::uint64_t next_id_ = 0;
	std::vector<std::pair<id::Soul, SoulName>> souls_;
	std::vector<Embodiment> embodiments_;
	std::vector<std::pair<id::Abode, AbodeName>> abode_rows_;
	mutable std::vector<Letter> letters_;
	std::vector<Supplication> supplications_;
	std::vector<ObedienceRow> obediences_;
	std::vector<DeedRow> deed_rows_;
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
