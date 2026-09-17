#pragma once

#include "beings/earth.h"
#include "beings/abode.h"
#include "acts/obedience.h"
#include "beings/soul.h"
#include "beings/heaven.h"
#include "beings/letter.h"
#include "beings/man.h"
#include "acts/supplication.h"
#include "beings/vessel.h"
#include "beings/world.h"
#include "identity/abode.h"
#include "identity/man.h"
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

	std::vector<Soul> souls() override { return souls_; }

	Soul enroll(const SoulName name) override
	{
		const id::Soul soul_id{++next_soul_id_};
		souls_.emplace_back(soul_id, name);
		return Soul{soul_id, name};
	}

	Embodiment embody(const id::Soul soul, DeviceToken token) override
	{
		const SoulName* name = nullptr;
		for (const Soul& row : souls_) {
			if (row.id() == soul) {
				name = &row.name();
				break;
			}
		}
		if (!name)
			throw std::invalid_argument("unknown soul");

		const id::Vessel vessel_id{++next_vessel_id_};
		const id::Man man_id{++next_man_id_};
		note_place(man_id.value());
		Embodiment row{man_id, soul, *name, vessel_id, std::move(token)};
		embodiments_.push_back(row);
		return row;
	}

	std::vector<Embodiment> embodiments() const override { return embodiments_; }

	/// Remember soul + embodiment before the living World wakes (Creation load).
	void seed_man(const id::Soul soul_id, const DeviceToken& token, const SoulName name)
	{
		souls_.emplace_back(soul_id, name);
		const id::Vessel vessel_id{soul_id.value()};
		const id::Man man_id{soul_id.value()};
		embodiments_.push_back(Embodiment{man_id, soul_id, name, vessel_id, token});
		if (soul_id.value() > next_soul_id_)
			next_soul_id_ = soul_id.value();
		if (vessel_id.value() > next_vessel_id_)
			next_vessel_id_ = vessel_id.value();
		if (man_id.value() > next_man_id_)
			next_man_id_ = man_id.value();
		note_place(man_id.value());
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

	void join_abode(const id::Abode abode, const id::Man man) override
	{
		for (const auto& row : abode_men_) {
			if (row.first == abode && row.second == man)
				return;
		}
		abode_men_.emplace_back(abode, man);
	}

	std::vector<std::pair<id::Abode, id::Man>> abode_men() override { return abode_men_; }

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
		return Obedience{oid, row.addressee().id(), row.suppliant().id(), true};
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
			if (row.id == id)
				return Obedience{row.id, row.testator, row.executor, row.living};
		}
		throw std::invalid_argument("unknown obedience");
	}

	void secede(const id::Obedience id) override
	{
		for (auto& row : obediences_) {
			if (row.id != id)
				continue;
			if (!row.living)
				throw std::logic_error("obedience is not living");
			row.living = false;
			return;
		}
		throw std::invalid_argument("unknown obedience");
	}

	FakeTime& fake_time() { return time_; }

private:
	struct ObedienceRow {
		id::Obedience id;
		id::Soul testator;
		id::Soul executor;
		bool living;
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
	std::uint64_t next_man_id_ = 0;
	std::uint64_t next_place_id_ = 0;
	std::uint64_t next_supplication_id_ = 0;
	mutable std::uint64_t next_id_ = 0;
	std::vector<Soul> souls_;
	std::vector<Embodiment> embodiments_;
	std::vector<std::pair<id::Abode, AbodeName>> abode_rows_;
	std::vector<std::pair<id::Abode, id::Man>> abode_men_;
	mutable std::vector<Letter> letters_;
	std::vector<Supplication> supplications_;
	std::vector<ObedienceRow> obediences_;
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
