#pragma once

#include "identity/word.h"
#include "matter/dating.h"
#include "matter/effort.h"
#include "matter/placement.h"
#include "matter/training.h"
#include "matter/word.h"

#include <optional>
#include <vector>


namespace will::domain::matter {


/// Gathered from the three dimensions; no single one keeps it.
class Behest {
public:
	Behest(Word word, Placement placement, Dating dating, std::optional<Training> training = std::nullopt,
		   std::vector<Effort> efforts = {});

	id::Word id() const noexcept { return word_.id(); }
	const Word& word() const noexcept { return word_; }
	const Placement& placement() const noexcept { return placement_; }
	const Dating& dating() const noexcept { return dating_; }

	const std::optional<Training>& training() const noexcept { return training_; }

	const std::vector<Effort>& efforts() const noexcept { return efforts_; }

private:
	Word word_;
	Placement placement_;
	Dating dating_;
	std::optional<Training> training_;
	std::vector<Effort> efforts_;
};


} // namespace will::domain::matter
