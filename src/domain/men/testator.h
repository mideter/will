#pragma once

#include "values/exercise.h"

#include "matter/man.h"
#include "matter/supplication.h"
#include "properties/birth.h"
#include "places/shepherding.h"
#include "relations/supplication.h"
#include "men/novice.h"
#include "words/behest.h"
#include "values/saying.h"

#include <functional>
#include <memory>
#include <vector>


namespace will::domain {


class Cell;
class Training;
class Word;


class World;


/// Testator (Тренер) — the side of a tie who wills its behests and trainings, and hears
/// the supplications that would bind one.
class Testator : public Novice {
public:
	Testator(Birth<World>, matter::Man kept);

	const Shepherding& accept(const Supplication& ask) const;
	void reject(const Supplication& ask) const;

	std::shared_ptr<const Behest> will(Birth<Cell> cell, const Shepherding& shepherding, const Saying& saying) const;

	/// In the cell of a tie one stands in.
	std::shared_ptr<const Training> train(const Saying& title, std::vector<Exercise> exercises) const;

	std::shared_ptr<const Training> train(Birth<Cell> cell, const Shepherding& shepherding, const Saying& title,
										  std::vector<Exercise> exercises) const;

	const Shepherding& shepherding(const Novice& novice) const;
	std::shared_ptr<const Supplication> supplication(const Novice& suppliant) const;
	std::vector<std::shared_ptr<const Supplication>> supplications() const;

	/// Heaven keeps the supplication until it is answered.
	void hear(Birth<Novice> birth) const;

	void hear(Birth<World> birth, matter::Supplication kept) const;

private:
	friend class World;
	friend class Supplication;
	friend class Tie;

	const Shepherding& shepherd(const Shepherding& place) const;
	void release(const Shepherding& place) const;

	void hold(std::shared_ptr<const Supplication> supplication) const;
	void drop(const Supplication& answered) const;

	mutable std::vector<const Shepherding*> shepherdings_;
};


} // namespace will::domain
