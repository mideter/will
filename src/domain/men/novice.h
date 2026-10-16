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
#include "men/witness.h"
#include "identity/word.h"

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>


namespace will::domain {


class Training;


class Testator;


/// Novice (Послушник) — the side of a tie who fulfils its behests and does its trainings.
class Novice : public Witness {
public:
	/// Ask the addressee to become one's testator.
	void supplicate(const Testator& addressee) const;

	/// Without a report the deed says «совершено».
	std::shared_ptr<const Deed> execute(const Behest& behest,
										std::optional<Saying> report = std::nullopt,
										std::optional<std::vector<Exercise>> performed = std::nullopt) const;

	struct Underway {
		id::Word training;
		std::uint32_t exercise;
		std::uint32_t approach;
		Timestamp begun;
	};

	/// Exercises are done in any order, the approaches of an exercise in order; once
	/// the willed are done, one beyond them.
	void begin(const Training& training, std::uint32_t exercise, std::uint32_t approach) const;

	matter::Effort finish(const Training& training, Weight weight, std::uint32_t repetitions) const;

	std::optional<Underway> underway() const;

	std::vector<std::reference_wrapper<const Obedience>> obediences() const;

	const Obedience& obedience(const Testator& testator) const;

protected:
	explicit Novice(matter::Man kept);

private:
	friend class Supplication;
	friend class World;

	const Obedience& follow(matter::Tie kept) const;
	const Obedience& follow(std::unique_ptr<Obedience> place) const;

	mutable std::vector<std::unique_ptr<Obedience>> obediences_;
	mutable std::mutex underway_mutex_;
	mutable std::optional<Underway> underway_;
};


} // namespace will::domain
