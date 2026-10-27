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


class Room;
class Novice;
class Training;
class Helm;
class Witness;


class World;


/// The side of a tie who wills its behests and trainings, and hears the supplications that
/// would bind one, refusing those he may not hear. Seeing his tie, as he looks at it or as it
/// is fulfilled before his eyes, he reviews its deeds. What he awaits are the behests of his
/// ties not yet fulfilled; what he has to review, those fulfilled whose deeds he has not yet
/// seen; each oldest first by tie.
class Testator : public virtual Embodied {
public:
	const Shepherding& accept(Birth<Helm> helm, const Supplication& ask) const;
	void reject(Birth<Helm> helm, const Supplication& ask) const;

	std::shared_ptr<const Behest> will(Birth<Room> room, const Shepherding& shepherding, const Saying& saying) const;

	std::shared_ptr<const Training> train(Birth<Room> room, const Shepherding& shepherding, const Saying& title,
										  std::vector<Exercise> exercises) const;

	void review(Birth<Witness> witness, const Shepherding& shepherding) const;
	void review(Birth<Novice> novice, const Shepherding& shepherding) const;

	const Shepherding& shepherding(const Novice& novice) const;

	std::vector<std::shared_ptr<const Behest>> awaited() const;
	std::vector<std::shared_ptr<const Behest>> unreviewed() const;
	std::shared_ptr<const Supplication> supplication(const Novice& suppliant) const;
	std::vector<std::shared_ptr<const Supplication>> supplications() const;

	void hear(Birth<Novice> birth) const;

protected:
	Testator() {}

private:
	friend class World;
	friend class Supplication;
	friend class Tie;

	const Shepherding& shepherd(const Shepherding& place) const;
	void review(const Shepherding& shepherding) const;
	void release(const Shepherding& place) const;


	mutable std::vector<const Shepherding*> shepherdings_;
};


} // namespace will::domain
