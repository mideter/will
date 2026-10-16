#pragma once

#include "places/obedience.h"
#include "places/shepherding.h"
#include "matter/tie.h"

#include "properties/birth.h"


namespace will::domain {


class Behest;
class Deed;
class Novice;
class Testator;


/// Tie (Узы) — the place a testator and a novice share: Послушание to the one, Ведение
/// to the other.
class Tie : public Obedience, public Shepherding {
public:
	Tie(Birth<Novice>, matter::Tie kept);
	~Tie() override;

	const Testator& testator() const override;
	const Novice& novice() const override;

	std::vector<std::shared_ptr<const Behest>> behests(const Novice& asker) const override;
	std::vector<std::shared_ptr<const Deed>> deeds(const Novice& asker) const override;

	std::shared_ptr<const Deed> inscribe(Birth<Novice>, matter::Deed kept) const override;
	std::shared_ptr<const Behest> inscribe(Birth<Testator>, matter::Behest kept) const override;

	bool dwells(const Man& man) const override;

	const Man& counterpart(const Man& side) const;

	/// A tie takes its testator's words, as behests.
	bool takes_words_of(const Man& writer) const override;
	std::shared_ptr<const Word> receive(const Contemplation& gaze, const Saying& saying) const override;
};


} // namespace will::domain
