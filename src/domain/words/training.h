#pragma once

#include "matter/behest.h"
#include "matter/effort.h"
#include "properties/birth.h"
#include "values/exercise.h"
#include "words/behest.h"

#include <memory>
#include <mutex>
#include <vector>


namespace will::domain {


class Life;
class Novice;
class Tie;


/// Training (Тренировка) — a behest that wills exercises: each done in approaches,
/// each approach with its weight, repetitions and the rest after it. Its word is
/// the testator's title or remark. The novice does it approach by approach — each
/// done is an effort it holds — and fulfils it by a deed.
class Training final : public Behest {
public:
	/// As a behest; the matter must carry the exercises.
	Training(Birth<Tie> birth, matter::Behest kept);
	Training(Birth<Life> birth, matter::Behest kept);

	const std::vector<Exercise>& exercises() const noexcept { return exercises_; }

	/// The approaches done so far, in the order they were finished.
	std::vector<matter::Effort> efforts() const;

	/// Whether this approach is already done.
	bool done(std::uint32_t exercise, std::uint32_t approach) const;

	/// An approach is done: the novice tells it, and it is held here for all who look.
	void exert(Birth<Novice> birth, matter::Effort kept) const;

private:
	static std::vector<Exercise> exercises_of(const matter::Behest& kept);

	std::vector<Exercise> exercises_;
	mutable std::unique_ptr<std::mutex> mutex_ = std::make_unique<std::mutex>();
	mutable std::vector<matter::Effort> efforts_;
};


} // namespace will::domain
