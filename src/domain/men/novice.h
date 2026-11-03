#pragma once

#include "values/exercise.h"
#include "values/weight.h"
#include "values/timestamp.h"
#include "matter/effort.h"

#include "matter/man.h"
#include "places/obedience.h"
#include "relations/supplication.h"
#include "matter/tie.h"
#include "words/behest.h"
#include "words/deed.h"
#include "values/saying.h"
#include "men/embodied.h"
#include "identity/word.h"

#include <functional>
#include <memory>
#include <optional>
#include <vector>


namespace will::domain {


class Acquaintance;
class Room;
class Training;


class Man;
class Testator;
class Helm;


class Novice : public virtual Embodied {
public:
	void supplicate(Birth<Helm> helm, const Acquaintance& dweller) const;

	std::shared_ptr<const Deed> execute(Birth<Room> room, const Behest& behest, std::optional<Saying> report,
										std::optional<std::vector<Exercise>> performed) const;

	void begin(Birth<Room> room, const Training& training, std::uint32_t exercise, std::uint32_t approach) const;

	void end(Birth<Room> room, const Training& training) const;

	matter::Effort finish(Birth<Room> room, const Training& training, Weight weight, std::uint32_t repetitions,
						  Strain strain) const;

	matter::Remark remark(Birth<Room> room, const Training& training, std::uint32_t exercise, const Saying& saying) const;

	std::vector<std::reference_wrapper<const Obedience>> obediences() const;

	const Obedience& obedience(const Testator& testator) const;

	std::vector<std::shared_ptr<const Behest>> owed() const;

protected:
	Novice() {}

private:
	friend class Supplication;
	friend class World;

	const Obedience& follow(matter::Tie kept) const;
	const Obedience& follow(std::unique_ptr<Obedience> place) const;

	mutable std::vector<std::unique_ptr<Obedience>> obediences_;
};


} // namespace will::domain
