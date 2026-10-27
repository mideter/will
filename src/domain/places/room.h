#pragma once

#include "matter/room.h"
#include "matter/effort.h"
#include "matter/remark.h"
#include "places/place.h"
#include "properties/birth.h"
#include "values/exercise.h"
#include "values/weight.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>


namespace will::domain {


class Abode;
class OuterAbode;
class Behest;
class Contemplation;
class Deed;
class Man;
class Novice;
class Recollection;
class Training;
class Word;


/// The window of an Abode onto the words of a place: of its abode, «Слово» in the interface;
/// of a tie, «Ведение» or «Послушание». It has no words of its own: it shows those of the
/// place it reflects to whom its abode shows it, and a gaze into it keeps them living. Only he writes here, and the place it reflects decides what his words
/// become; in the novice's room of a tie, the novice fulfils, trains and remarks on the
/// exercises.
class Room final : public Place {
public:
	Room(Birth<Abode> birth, const Place& reflects, matter::Room kept);

	const Abode& abode() const noexcept { return abode_; }
	const Place& reflects() const noexcept { return reflects_; }

	bool leads_to(const Man& who, const Place& next) const override;

	bool within(const OuterAbode& abode) const override;

	std::shared_ptr<const Recollection> recollection(Birth<Contemplation> gaze) const;

	std::vector<std::shared_ptr<const Word>> words(const Contemplation& gaze) const;

	bool writable_by(const Man& writer) const;

	std::shared_ptr<const Word> write(const Contemplation& gaze, const Saying& saying) const;

	std::shared_ptr<const Training> train(const Contemplation& gaze, const Saying& title,
										  std::vector<Exercise> exercises) const;

	std::shared_ptr<const Deed> fulfil(const Contemplation& gaze, const Behest& behest, std::optional<Saying> report,
									  std::optional<std::vector<Exercise>> performed) const;
	void begin(const Contemplation& gaze, const Training& training, std::uint32_t exercise,
			   std::uint32_t approach) const;
	void end(const Contemplation& gaze, const Training& training) const;
	matter::Effort finish(const Contemplation& gaze, const Training& training, Weight weight,
						  std::uint32_t repetitions, Strain strain) const;
	matter::Remark remark(const Contemplation& gaze, const Training& training, std::uint32_t exercise,
						  const Saying& saying) const;

private:
	template<typename OtherTie, typename NotNovice>
	const Novice& obeying(const Contemplation& gaze, const Behest& behest) const;

	const Abode& abode_;
	const Place& reflects_;
};


} // namespace will::domain
