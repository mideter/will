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


class Acquaintance;
class Cell;
class Training;


class Man;
class Testator;
class Helm;


/// The side of a tie who fulfils its behests and does its trainings, and asks to be led.
/// Without a report his deed says «совершено»; a deed of a training tells what was done,
/// the exercises in their willed order, those of which nothing was done left out. He does
/// the exercises in any order, the approaches of an exercise in order, and once the willed
/// are done, one beyond them; one approach of a training at a time. He ends an approach, and
/// then tells what was done in it; until he tells, he begins no other. Until the training is
/// fulfilled he may say remarks on its exercises, as many as he will.
class Novice : public virtual Embodied {
public:
	void supplicate(Birth<Helm> helm, const Acquaintance& dweller) const;

	std::shared_ptr<const Deed> execute(Birth<Cell> cell, const Behest& behest, std::optional<Saying> report,
										std::optional<std::vector<Exercise>> performed) const;

	void begin(Birth<Cell> cell, const Training& training, std::uint32_t exercise, std::uint32_t approach) const;

	void end(Birth<Cell> cell, const Training& training) const;

	matter::Effort finish(Birth<Cell> cell, const Training& training, Weight weight, std::uint32_t repetitions,
						  Strain strain) const;

	matter::Remark remark(Birth<Cell> cell, const Training& training, std::uint32_t exercise, const Saying& saying) const;

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
