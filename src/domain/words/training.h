#pragma once

#include "matter/behest.h"
#include "properties/birth.h"
#include "values/exercise.h"
#include "words/behest.h"

#include <vector>


namespace will::domain {


class Life;
class Tie;


/// Training (Тренировка) — a behest that wills exercises: each done in approaches,
/// each approach with its weight and repetitions. Its word is the testator's title
/// or remark. The novice fulfils it by a deed telling what he has done.
class Training final : public Behest {
public:
	/// As a behest; the matter must carry the exercises.
	Training(Birth<Tie> birth, matter::Behest kept);
	Training(Birth<Life> birth, matter::Behest kept);

	const std::vector<Exercise>& exercises() const noexcept { return exercises_; }

private:
	static std::vector<Exercise> exercises_of(const matter::Behest& kept);

	std::vector<Exercise> exercises_;
};


} // namespace will::domain
