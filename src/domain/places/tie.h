#pragma once

#include "places/obedience.h"
#include "places/shepherding.h"
#include "matter/tie.h"

#include "properties/birth.h"
#include "values/exercise.h"
#include "values/saying.h"


namespace will::domain {


class Behest;
class Cell;
class Deed;
class Novice;
class Testator;
class Training;


/// Tie (Узы) — the place a testator and a novice share: Послушание to the one, Ведение
/// to the other.
class Tie : public Obedience, public Shepherding {
public:
	Tie(Birth<Novice>, matter::Tie kept);
	~Tie() override;

	const Man& testator() const override;
	const Man& novice() const override;

	std::vector<std::shared_ptr<const Behest>> behests(const Novice& asker) const override;
	std::vector<std::shared_ptr<const Deed>> deeds(const Novice& asker) const override;

	std::shared_ptr<const Deed> inscribe(Birth<Novice>, matter::Deed kept) const override;
	std::shared_ptr<const Behest> inscribe(Birth<Testator>, matter::Behest kept) const override;

	bool dwells(const Man& man) const override;

	const Man& counterpart(const Man& side) const;

	/// A tie takes its testator's words, as behests.
	bool takes_words_of(const Man& host) const override;
	std::shared_ptr<const Word> receive(Birth<Cell> cell, const Saying& saying) const override;

	std::shared_ptr<const Training> train(Birth<Cell> cell, const Saying& title, std::vector<Exercise> exercises) const;

private:
	/// The testator, if the cell is his.
	const Testator& willing(Birth<Cell> cell) const;
};


} // namespace will::domain
