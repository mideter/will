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


/// A behest that wills exercises; the novice does it approach by approach, one underway at
/// a time — ended, it waits to be told what was done — and fulfils it by a deed. Its efforts come in the order they were finished, his
/// remarks on its exercises in the order they were said.
class Training final : public Behest {
public:
	Training(Birth<Tie> birth, matter::Behest kept);
	Training(Birth<Life> birth, matter::Behest kept);

	const std::vector<Exercise>& exercises() const noexcept { return exercises_; }

	std::vector<matter::Effort> efforts() const;

	bool done(std::uint32_t exercise, std::uint32_t approach) const;

	std::optional<matter::Underway> underway() const;

	void begin(Birth<Novice> birth, matter::Underway kept) const;

	void end(Birth<Novice> birth, matter::Underway kept) const;

	void exert(Birth<Novice> birth, matter::Effort kept) const;

	std::vector<matter::Remark> remarks() const;

	void remark(Birth<Novice> birth, matter::Remark kept) const;

private:
	static std::vector<Exercise> exercises_of(const matter::Behest& kept);

	std::vector<Exercise> exercises_;
	mutable std::unique_ptr<std::mutex> mutex_ = std::make_unique<std::mutex>();
	mutable std::vector<matter::Effort> efforts_;
	mutable std::optional<matter::Underway> underway_;
	mutable std::vector<matter::Remark> remarks_;
};


} // namespace will::domain
