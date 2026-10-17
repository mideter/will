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


/// Training (Тренировка) — a behest that wills exercises; the novice does it approach by
/// approach and fulfils it by a deed.
class Training final : public Behest {
public:
	Training(Birth<Tie> birth, matter::Behest kept);
	Training(Birth<Life> birth, matter::Behest kept);

	const std::vector<Exercise>& exercises() const noexcept { return exercises_; }

	/// In the order they were finished.
	std::vector<matter::Effort> efforts() const;

	bool done(std::uint32_t exercise, std::uint32_t approach) const;

	/// One at a time.
	std::optional<matter::Underway> underway() const;

	void begin(Birth<Novice> birth, matter::Underway kept) const;

	/// The approach underway is done.
	void exert(Birth<Novice> birth, matter::Effort kept) const;

private:
	static std::vector<Exercise> exercises_of(const matter::Behest& kept);

	std::vector<Exercise> exercises_;
	mutable std::unique_ptr<std::mutex> mutex_ = std::make_unique<std::mutex>();
	mutable std::vector<matter::Effort> efforts_;
	mutable std::optional<matter::Underway> underway_;
};


} // namespace will::domain
