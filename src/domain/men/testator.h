#pragma once

#include "values/exercise.h"

#include "matter/supplication.h"
#include "properties/birth.h"
#include "places/shepherding.h"
#include "relations/supplication.h"
#include "men/embodied.h"
#include "words/behest.h"
#include "values/saying.h"

#include <memory>
#include <vector>


namespace will::domain {


class Cell;
class Novice;
class Training;
class Helm;


class World;


/// Testator (Тренер) — the side of a tie who wills its behests and trainings, and hears
/// the supplications that would bind one.
class Testator : public virtual Embodied {
public:
	const Shepherding& accept(Birth<Helm> helm, const Supplication& ask) const;
	void reject(Birth<Helm> helm, const Supplication& ask) const;

	std::shared_ptr<const Behest> will(Birth<Cell> cell, const Shepherding& shepherding, const Saying& saying) const;

	std::shared_ptr<const Training> train(Birth<Cell> cell, const Shepherding& shepherding, const Saying& title,
										  std::vector<Exercise> exercises) const;

	const Shepherding& shepherding(const Novice& novice) const;
	std::shared_ptr<const Supplication> supplication(const Novice& suppliant) const;
	std::vector<std::shared_ptr<const Supplication>> supplications() const;

	/// Whether he may hear the suppliant: he refuses if not.
	void hear(Birth<Novice> birth) const;

protected:
	Testator() {}

private:
	friend class World;
	friend class Supplication;
	friend class Tie;

	const Shepherding& shepherd(const Shepherding& place) const;
	void release(const Shepherding& place) const;


	mutable std::vector<const Shepherding*> shepherdings_;
};


} // namespace will::domain
