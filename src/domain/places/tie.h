#pragma once

#include "places/obedience.h"
#include "places/shepherding.h"
#include "matter/tie.h"

#include "properties/birth.h"
#include "values/exercise.h"
#include "values/saying.h"


namespace will::domain {


class Abode;
class Behest;
class Room;
class Deed;
class Novice;
class Testator;
class Training;


/// The place a testator and a novice share: Послушание to the one, Ведение to the other.
/// It takes its testator's words, as behests.
class Tie : public Obedience, public Shepherding {
public:
	Tie(Birth<Novice>, matter::Tie kept);
	~Tie() override;

	const Man& testator() const override;
	const Man& novice() const override;

	std::vector<std::shared_ptr<const Behest>> behests(const Contemplation& gaze) const override;
	std::vector<std::shared_ptr<const Deed>> deeds(const Contemplation& gaze) const override;

	std::vector<std::shared_ptr<const Behest>> behests(Birth<Novice> novice) const override;
	std::vector<std::shared_ptr<const Behest>> behests(Birth<Testator> testator) const override;

	std::shared_ptr<const Deed> inscribe(Birth<Novice>, const Behest& behest, matter::Deed kept) const override;
	std::shared_ptr<const Behest> inscribe(Birth<Testator>, matter::Behest kept) const override;

	bool dwells(const Man& man) const override;

	const Man& counterpart(const Man& side) const;

	bool takes_words_of(const Man& host) const override;
	std::shared_ptr<const Word> receive(Birth<Room> room, const Saying& saying) const override;

	std::shared_ptr<const Training> train(Birth<Room> room, const Saying& title, std::vector<Exercise> exercises) const;

private:
	std::vector<std::shared_ptr<const Word>> words(const Contemplation& gaze) const;
	std::vector<std::shared_ptr<const Behest>> unbeheld() const;

	const Testator& testator_of(Birth<Room> room) const;
};


} // namespace will::domain
