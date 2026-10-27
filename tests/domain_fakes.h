#pragma once

#include "matter/dating.h"
#include "matter/abode.h"
#include "matter/supplication.h"
#include "matter/execution.h"
#include "matter/review.h"
#include "matter/placement.h"
#include "matter/tie.h"
#include "places/obedience.h"
#include "places/tie.h"
#include "matter/word.h"
#include "relations/supplication.h"
#include "places/abode.h"
#include "words/letter.h"
#include "places/room.h"
#include "places/gates.h"
#include "places/helm.h"
#include "places/part.h"
#include "men/witness.h"
#include "words/recollection.h"
#include "relations/contemplation.h"
#include "men/soul.h"
#include "horizons/creation.h"
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
#include <algorithm>
#include <optional>
#include <stdexcept>
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

	std::vector<std::pair<id::Soul, SoulName>> souls;
	std::vector<matter::Word> words;
	std::vector<matter::Fatherhood> fatherhoods;
	std::vector<matter::Training> trainings;
	std::vector<matter::Remarked> remarks;

	std::vector<std::pair<id::Soul, matter::Abode>> abodes;
	std::vector<matter::Tie> ties;
	std::vector<matter::Dweller> dwellers;
	std::vector<matter::Room> rooms;
	std::vector<matter::Placement> placements;

	std::vector<matter::Vessel> vessels;
	std::vector<matter::Embodiment> embodiments;
	std::vector<matter::Dating> datings;
	std::vector<matter::Supplication> supplications;  // awaiting their answer
	std::vector<matter::Answer> answers;
	std::vector<matter::Execution> executions;
	std::vector<matter::Review> reviews;
	std::vector<matter::Effort> efforts;
	std::vector<matter::Underway> underways;
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

	matter::Fatherhood father(const id::Soul father, const id::Soul child, const matter::Fatherhood::Line line) override
	{
		const matter::Fatherhood kept{father, child, line};
		for (matter::Fatherhood& row : shared_.fatherhoods) {
			if (row.child() != child || row.line() != line)
				continue;
			if (line == matter::Fatherhood::Line::Flesh)
				throw std::logic_error("the child already has his father by flesh");
			row = kept;
			return kept;
		}
		shared_.fatherhoods.push_back(kept);
		return kept;
	}

	std::vector<matter::Fatherhood> fatherhoods() const override { return shared_.fatherhoods; }

	matter::Training utter(const id::Word word, std::vector<Exercise> exercises) override
	{
		for (const matter::Training& row : shared_.trainings) {
			if (row.id() == word)
				throw std::logic_error("the word already has its exercises");
		}
		shared_.trainings.emplace_back(word, std::move(exercises));
		return shared_.trainings.back();
	}

	std::vector<matter::Training> trainings(const std::vector<id::Word>& ids) const override
	{
		std::vector<matter::Training> out;
		for (const id::Word id : ids) {
			for (const matter::Training& row : shared_.trainings) {
				if (row.id() == id)
					out.push_back(row);
			}
		}
		return out;
	}

	matter::Remarked remark(const id::Word remark, const id::Word training, const std::uint32_t exercise) override
	{
		for (const matter::Remarked& row : shared_.remarks) {
			if (row.id() == remark)
				throw std::logic_error("the word is already a remark");
		}
		shared_.remarks.emplace_back(remark, training, exercise);
		return shared_.remarks.back();
	}

	std::vector<matter::Remarked> remarked(const std::vector<id::Word>& trainings) const override
	{
		std::vector<matter::Remarked> out;
		for (const id::Word training : trainings) {
			for (const matter::Remarked& row : shared_.remarks) {
				if (row.training() == training)
					out.push_back(row);
			}
		}
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

		const id::Place id = eternity_.space().point();
		const id::Place gates = eternity_.space().point();
		const matter::Abode kept{id, std::move(name), gates, eternity_.space().point()};
		shared_.abodes.emplace_back(host, kept);
		return kept;
	}

	matter::Dweller dwell(const id::Place abode, const id::Soul soul, const matter::Dweller::Kind kind) override
	{
		const matter::Dweller kept{abode, soul, kind};
		for (matter::Dweller& row : shared_.dwellers) {
			if (row.abode() == abode && row.soul() == soul) {
				row = kept;
				return kept;
			}
		}
		shared_.dwellers.push_back(kept);
		return kept;
	}

	std::vector<matter::Dweller> dwellers() const override { return shared_.dwellers; }

	matter::Room furnish(const id::Place abode, const id::Place reflects) override
	{
		for (const matter::Room& row : shared_.rooms) {
			if (row.abode() == abode && row.reflects() == reflects)
				throw std::logic_error("the abode already has a room reflecting this place");
		}
		shared_.rooms.emplace_back(eternity_.space().point(), abode, reflects, matter::Room::Part::Inner);
		return shared_.rooms.back();
	}

	matter::Room arrange(const id::Place room, const matter::Room::Part part) override
	{
		for (matter::Room& row : shared_.rooms) {
			if (row.id() == room) {
				row = matter::Room{row.id(), row.abode(), row.reflects(), part};
				return row;
			}
		}
		throw std::invalid_argument("unknown room");
	}

	std::vector<matter::Room> rooms(const id::Place abode) const override
	{
		std::vector<matter::Room> out;
		for (const matter::Room& row : shared_.rooms) {
			if (row.abode() == abode)
				out.push_back(row);
		}
		return out;
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

	std::size_t placement_reads() const noexcept { return placement_reads_; }

	std::vector<matter::Placement> placements(const id::Place place) const override
	{
		++placement_reads_;

		std::vector<matter::Placement> matching;
		for (const matter::Placement& row : shared_.placements) {
			if (row.place() == place)
				matching.push_back(row);
		}
		return matching;
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

	matter::Vessel form(DeviceToken token) override
	{
		shared_.vessels.emplace_back(id::Vessel{++shared_.next_vessel_id}, std::move(token));
		return shared_.vessels.back();
	}

	matter::Embodiment embody(const id::Soul soul, const id::Vessel vessel) override
	{
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

	matter::Answer answer(const id::Soul suppliant, const id::Soul addressee, const matter::Answer::Form form) override
	{
		const auto it = awaiting(suppliant, addressee);
		if (it == shared_.supplications.end())
			throw std::invalid_argument("unknown supplication");

		shared_.supplications.erase(it);
		shared_.answers.emplace_back(suppliant, addressee, form);
		return shared_.answers.back();
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

	matter::Underway begin(const id::Word training, const std::uint32_t exercise, const std::uint32_t approach) override
	{
		for (const matter::Underway& row : shared_.underways) {
			if (row.training() == training)
				throw std::logic_error("another approach of the training is underway");
		}
		shared_.underways.emplace_back(training, exercise, approach, eternity_.time().instant());
		return shared_.underways.back();
	}

	std::vector<matter::Underway> underways(const std::vector<id::Word>& trainings) const override
	{
		std::vector<matter::Underway> out;
		for (const id::Word training : trainings) {
			for (const matter::Underway& row : shared_.underways) {
				if (row.training() == training)
					out.push_back(row);
			}
		}
		return out;
	}

	matter::Underway end(const matter::Underway& doing) override
	{
		for (matter::Underway& row : shared_.underways) {
			if (row.training() == doing.training() && row.exercise() == doing.exercise()
				&& row.approach() == doing.approach() && row.begun() == doing.begun()) {
				if (row.ended())
					throw std::logic_error("the approach has already ended");
				row = matter::Underway{row.training(), row.exercise(), row.approach(), row.begun(), eternity_.time().instant()};
				return row;
			}
		}
		throw std::logic_error("the approach is not underway");
	}

	matter::Effort exert(const matter::Underway& doing, const Weight weight, const std::uint32_t repetitions,
						 const Strain strain) override
	{
		const auto underway = std::find_if(shared_.underways.begin(), shared_.underways.end(), [&](const auto& row) {
			return row.training() == doing.training() && row.exercise() == doing.exercise()
				   && row.approach() == doing.approach() && row.begun() == doing.begun();
		});
		if (underway == shared_.underways.end())
			throw std::logic_error("the approach is not underway");
		for (const matter::Effort& row : shared_.efforts) {
			if (row.training() == doing.training() && row.exercise() == doing.exercise()
				&& row.approach() == doing.approach())
				throw std::logic_error("the approach is already done");
		}
		shared_.efforts.emplace_back(doing.training(), doing.exercise(), doing.approach(), weight, repetitions,
									 doing.begun(), underway->ended().value_or(eternity_.time().instant()), strain);
		shared_.underways.erase(underway);
		return shared_.efforts.back();
	}

	std::vector<matter::Effort> efforts(const std::vector<id::Word>& trainings) const override
	{
		std::vector<matter::Effort> out;
		for (const id::Word training : trainings) {
			for (const matter::Effort& row : shared_.efforts) {
				if (row.training() == training)
					out.push_back(row);
			}
		}
		return out;
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

	matter::Review review(const id::Word deed) override
	{
		for (const matter::Review& row : shared_.reviews) {
			if (row.id() == deed)
				throw std::logic_error("deed is already reviewed");
		}
		shared_.reviews.emplace_back(deed, eternity_.time().instant());
		return shared_.reviews.back();
	}

	std::vector<matter::Review> reviews(const std::vector<id::Word>& deeds) const override
	{
		std::vector<matter::Review> out;
		for (const matter::Review& row : shared_.reviews) {
			if (std::find(deeds.begin(), deeds.end(), row.id()) != deeds.end())
				out.push_back(row);
		}
		return out;
	}

private:
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


inline const Contemplation& gaze_of(World& world, const Man& man)
{
	const std::shared_ptr<const Contemplation> gaze = world.contemplation(man);
	if (!gaze)
		throw std::logic_error("he contemplates nothing");
	return *gaze;
}


inline std::shared_ptr<const Word> write(World& world, const Man& man, const Saying& saying)
{
	const Contemplation& gaze = gaze_of(world, man);
	return gaze.room().write(gaze, saying);
}


inline std::shared_ptr<const Behest> will_here(World& world, const Man& man, const Saying& saying)
{
	return std::dynamic_pointer_cast<const Behest>(write(world, man, saying));
}


inline bool writes_here(World& world, const Man& man)
{
	const std::shared_ptr<const Contemplation> gaze = world.contemplation(man);
	return gaze && gaze->writable();
}


inline std::shared_ptr<const Training> train(World& world, const Man& man, const Saying& title,
											 std::vector<Exercise> exercises)
{
	const Contemplation& gaze = gaze_of(world, man);
	return gaze.room().train(gaze, title, std::move(exercises));
}


inline std::shared_ptr<const Deed> execute(World& world, const Man& man, const Behest& behest,
										   std::optional<Saying> report = std::nullopt,
										   std::optional<std::vector<Exercise>> performed = std::nullopt)
{
	const Contemplation& gaze = gaze_of(world, man);
	return gaze.room().fulfil(gaze, behest, std::move(report), std::move(performed));
}


inline void begin(World& world, const Man& man, const Training& training, std::uint32_t exercise,
				  std::uint32_t approach)
{
	const Contemplation& gaze = gaze_of(world, man);
	gaze.room().begin(gaze, training, exercise, approach);
}


inline void end(World& world, const Man& man, const Training& training)
{
	const Contemplation& gaze = gaze_of(world, man);
	gaze.room().end(gaze, training);
}


inline matter::Effort finish(World& world, const Man& man, const Training& training, Weight weight,
							 std::uint32_t repetitions, Strain strain = Strain{})
{
	const Contemplation& gaze = gaze_of(world, man);
	return gaze.room().finish(gaze, training, weight, repetitions, strain);
}


inline matter::Remark remark(World& world, const Man& man, const Training& training, std::uint32_t exercise,
							 const Saying& saying)
{
	const Contemplation& gaze = gaze_of(world, man);
	return gaze.room().remark(gaze, training, exercise, saying);
}


inline void admit(World& world, const Man& keeper, const Man& man)
{
	const Contemplation& gaze = gaze_of(world, keeper);
	gaze.gates().admit(gaze, man);
}


inline void knock(World& world, const Man& man)
{
	const Contemplation& gaze = gaze_of(world, man);
	gaze.gates().knock(gaze);
}


inline const Man& bear(World& world, const Man& midwife, const Unborn& unborn)
{
	const Contemplation& gaze = gaze_of(world, midwife);
	return gaze.gates().bear(gaze, unborn);
}


inline void regard(World& world, const Man& man, const Man& dweller, matter::Dweller::Kind kind)
{
	const Contemplation& gaze = gaze_of(world, man);
	gaze.helm().regard(gaze, dweller, kind);
}


inline void arrange(World& world, const Man& man, const Room& room, const Part& part)
{
	const Contemplation& gaze = gaze_of(world, man);
	gaze.helm().arrange(gaze, room, part);
}


inline const Abode& abode_of(const Place& place)
{
	if (const auto* abode = dynamic_cast<const Abode*>(&place))
		return *abode;
	if (const auto* gates = dynamic_cast<const Gates*>(&place))
		return gates->abode();
	if (const auto* helm = dynamic_cast<const Helm*>(&place))
		return helm->abode();
	return dynamic_cast<const Room&>(place).abode();
}


inline void turn(const Man& man, const Place& place)
{
	const auto& witness = man;
	if (const auto* abode = dynamic_cast<const Abode*>(&place))
		witness.contemplate(*abode);
	else if (const auto* gates = dynamic_cast<const Gates*>(&place))
		witness.contemplate(*gates);
	else if (const auto* helm = dynamic_cast<const Helm*>(&place))
		witness.contemplate(*helm);
	else
		witness.contemplate(dynamic_cast<const Room&>(place));
}


inline void go(World& world, const Man& man, const Place& place)
{
	const std::shared_ptr<const Contemplation> gaze = world.contemplation(man);
	const Place& here = gaze ? gaze->place() : man.abode();
	if (!here.leads_to(place)) {
		turn(man, abode_of(here).outer().gates());
		turn(man, abode_of(place).outer().gates());
	}
	turn(man, place);
}


template<typename Act>
void looking_at(World& world, const Man& man, const Place& room, Act act)
{
	const std::shared_ptr<const Contemplation> before = world.contemplation(man);
	go(world, man, room);
	act();
	if (before)
		go(world, man, before->place());
	else
		man.sleep();
}


inline void admit_at_gates(World& world, const Man& host, const Man& man)
{
	looking_at(world, host, host.abode().outer().gates(), [&] {
		looking_at(world, man, host.abode().outer().gates(), [&] {
			knock(world, man);
			admit(world, host, man);
		});
	});
}


inline void regard_in_helm(World& world, const Man& host, const Man& man, const matter::Dweller::Kind kind)
{
	looking_at(world, host, host.abode().inner().helm(), [&] { regard(world, host, man, kind); });
}


inline void supplicate(World& world, const Man& man, const Man& addressee)
{
	looking_at(world, man, man.abode().inner().helm(), [&] {
		const Contemplation& gaze = gaze_of(world, man);
		gaze.helm().supplicate(gaze, addressee);
	});
}


inline const Shepherding& accept(World& world, const Man& testator, const Supplication& ask)
{
	const Shepherding* accepted = nullptr;
	looking_at(world, testator, testator.abode().inner().helm(), [&] {
		const Contemplation& gaze = gaze_of(world, testator);
		accepted = &gaze.helm().accept(gaze, ask);
	});
	return *accepted;
}


inline void reject(World& world, const Man& testator, const Supplication& ask)
{
	looking_at(world, testator, testator.abode().inner().helm(), [&] {
		const Contemplation& gaze = gaze_of(world, testator);
		gaze.helm().reject(gaze, ask);
	});
}


inline void arrange_in_helm(World& world, const Man& host, const Room& room, const Part& part)
{
	looking_at(world, host, host.abode().inner().helm(), [&] { arrange(world, host, room, part); });
}


inline const Room& room_of(const Man& man)
{
	return *man.abode().room(man.abode());
}


inline const Room& room_in(const Man& man, const Place& reflected)
{
	return *man.abode().room(reflected);
}


inline std::vector<std::shared_ptr<const Word>> words_seen_by(const std::shared_ptr<const Contemplation>& gaze)
{
	return gaze->room().words(*gaze);
}


inline std::vector<std::shared_ptr<const Letter>> letters_seen_by(const std::shared_ptr<const Contemplation>& gaze)
{
	std::vector<std::shared_ptr<const Letter>> letters;
	for (const std::shared_ptr<const Word>& word : words_seen_by(gaze)) {
		if (auto letter = std::dynamic_pointer_cast<const Letter>(word))
			letters.push_back(std::move(letter));
	}
	return letters;
}


inline constexpr std::string_view elder_token = "f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0";

inline const Man& forefather(World& world)
{
	return dynamic_cast<const Man&>(world.welcome(*DeviceToken::parse(elder_token)));
}


inline const Man& born(World& world, const DeviceToken& token)
{
	const Man& elder = forefather(world);
	const Vessel& body = world.welcome(token);
	if (const auto* man = dynamic_cast<const Man*>(&body))
		return *man;

	const Man* child = nullptr;
	looking_at(world, elder, elder.abode().outer().gates(),
			   [&] { child = &bear(world, elder, dynamic_cast<const Unborn&>(body)); });
	return *child;
}


inline const Soul& register_soul_with_vessel(World& world, const std::string_view device_token)
{
	return born(world, *DeviceToken::parse(device_token));
}


inline void seed_man(InMemoryCosmos& cosmos, const id::Soul soul_id, const DeviceToken& token, const SoulName name)
{
	InMemoryShared& shared = cosmos.shared();
	const id::Soul elder{900};
	if (soul_id != elder && std::none_of(shared.souls.begin(), shared.souls.end(),
										 [&](const auto& soul) { return soul.first == elder; })) {
		shared.souls.emplace_back(elder, *SoulName::parse("elder000"));
		shared.vessels.push_back(matter::Vessel{id::Vessel{900}, *DeviceToken::parse(elder_token)});
		shared.embodiments.emplace_back(elder, id::Vessel{900});
	}

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
