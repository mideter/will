#pragma once

#include "values/exercise.h"
#include "values/weight.h"
#include "values/timestamp.h"
#include "matter/effort.h"

#include "matter/man.h"
#include "places/obedience.h"
#include "relations/supplication.h"
#include "matter/tie.h"
#include "words/behest.h"
#include "words/deed.h"
#include "values/saying.h"
#include "men/embodied.h"
#include "identity/word.h"

#include <functional>
#include <memory>
#include <optional>
#include <vector>


namespace will::domain {


class Cell;
class Training;


class Man;
class Testator;


/// Novice (Послушник) — the side of a tie who fulfils its behests and does its trainings.
class Novice : public virtual Embodied {
public:
	/// Ask the addressee to become one's testator.
	void supplicate(const Man& addressee) const;

	/// Without a report the deed says «совершено».
	std::shared_ptr<const Deed> execute(Birth<Cell> cell, const Behest& behest, std::optional<Saying> report,
										std::optional<std::vector<Exercise>> performed) const;

	/// Exercises are done in any order, the approaches of an exercise in order; once
	/// the willed are done, one beyond them; one approach of a training at a time.
	void begin(Birth<Cell> cell, const Training& training, std::uint32_t exercise, std::uint32_t approach) const;

	matter::Effort finish(Birth<Cell> cell, const Training& training, Weight weight, std::uint32_t repetitions) const;

	std::vector<std::reference_wrapper<const Obedience>> obediences() const;

	const Obedience& obedience(const Testator& testator) const;

protected:
	Novice() {}

private:
	friend class Supplication;
	friend class World;

	const Obedience& follow(matter::Tie kept) const;
	const Obedience& follow(std::unique_ptr<Obedience> place) const;

	mutable std::vector<std::unique_ptr<Obedience>> obediences_;
};


} // namespace will::domain
