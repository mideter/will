#pragma once

#include "identity/word.h"
#include "matter/dating.h"
#include "matter/effort.h"
#include "matter/underway.h"
#include "matter/placement.h"
#include "matter/remark.h"
#include "matter/training.h"
#include "matter/word.h"

#include <optional>
#include <vector>


namespace will::domain::matter {


/// Gathered from the three dimensions; no single one keeps it.
class Behest {
public:
	Behest(Word word, Placement placement, Dating dating, std::optional<Training> training = std::nullopt,
		   std::vector<Effort> efforts = {}, std::optional<Underway> underway = std::nullopt,
		   std::vector<Remark> remarks = {});

	id::Word id() const noexcept { return word_.id(); }
	const Word& word() const noexcept { return word_; }
	const Placement& placement() const noexcept { return placement_; }
	const Dating& dating() const noexcept { return dating_; }

	const std::optional<Training>& training() const noexcept { return training_; }

	const std::vector<Effort>& efforts() const noexcept { return efforts_; }

	const std::optional<Underway>& underway() const noexcept { return underway_; }

	const std::vector<Remark>& remarks() const noexcept { return remarks_; }

private:
	Word word_;
	Placement placement_;
	Dating dating_;
	std::optional<Training> training_;
	std::vector<Effort> efforts_;
	std::optional<Underway> underway_;
	std::vector<Remark> remarks_;
};


} // namespace will::domain::matter
