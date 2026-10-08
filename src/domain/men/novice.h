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


/// Novice (Послушник) — Исполнитель: исполняет Веление в Послушании.
/// Owns living Ties as Obedience (Послушание) on the heap; Spatiality keeps
/// their matter::Tie. Mode is disclosed in an Obedience; the living heap object
/// is always Testator.
class Novice : public Witness {
public:
	/// Ask addressee to become Завещатель; this soul will be the novice. Only
	/// those who dwell in each other's abodes are tied: the addressee must dwell
	/// in this soul's abode, and this soul in his.
	/// The pending Supplication is signed by this novice (lodge + Temporality).
	void supplicate(const Testator& addressee) const;

	/// Fulfil a behest of a tie where this soul is novice, with a report or,
	/// without one, «совершено», while contemplating that tie. A training is
	/// fulfilled with the exercises done — as willed, when not told otherwise; a
	/// plain behest takes none. Throws if the behest is already fulfilled or one
	/// looks elsewhere.
	std::shared_ptr<const Deed> execute(const Behest& behest,
										std::optional<Saying> report = std::nullopt,
										std::optional<std::vector<Exercise>> performed = std::nullopt) const;

	/// The approach one is doing now: of which training, which one, begun when.
	struct Underway {
		id::Word training;
		std::uint32_t exercise;
		std::uint32_t approach;
		Timestamp begun;
	};

	/// Begin an approach of a training of a tie where this soul is novice, while
	/// contemplating that tie: an approach willed, or the next one beyond them.
	/// Throws if the training is fulfilled, the approach is done or is no such,
	/// or another approach is underway.
	void begin(const Training& training, std::uint32_t exercise, std::uint32_t approach) const;

	/// Finish the approach underway with the weight and repetitions done: the
	/// effort is kept and the training holds it. Throws if none is underway of
	/// this training, or one looks elsewhere.
	matter::Effort finish(const Training& training, Weight weight, std::uint32_t repetitions) const;

	/// The approach underway; none if one is doing none.
	std::optional<Underway> underway() const;

	std::vector<std::reference_wrapper<const Obedience>> obediences() const;

	/// Owned Obedience face under this testator. Throws if unknown.
	const Obedience& obedience(const Testator& testator) const;

protected:
	explicit Novice(matter::Man kept);

private:
	friend class Supplication;
	friend class World;

	using Witness::say;

	const Obedience& follow(matter::Tie kept) const;
	const Obedience& follow(std::unique_ptr<Obedience> place) const;

	mutable std::vector<std::unique_ptr<Obedience>> obediences_;
	mutable std::mutex underway_mutex_;
	mutable std::optional<Underway> underway_;
};


} // namespace will::domain
