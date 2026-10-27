#pragma once

#include "matter/dweller.h"
#include "matter/room.h"
#include "places/place.h"
#include "properties/birth.h"
#include "properties/immanent.h"

#include "matter/effort.h"
#include "matter/remark.h"
#include "values/exercise.h"
#include "values/weight.h"

#include <atomic>
#include <cstdint>
#include <optional>
#include <vector>
#include <memory>
#include <string>


namespace will::domain {


class Abode;
class Man;
class Heaven;
class Behest;
class Deed;
class Novice;
class Part;
class Shepherding;
class Supplication;
class Training;
class Unborn;


/// A window in an Abode onto a place, with no words of its own. It stands in a part of the
/// abode: a cell where its host sets it, the gates and the helm as the cores of theirs.
/// Others may look in, but one acts in a room only as its host.
class Room : public Place {
public:
	const Abode& abode() const noexcept { return abode_; }
	const Place& reflects() const noexcept { return reflects_; }
	matter::Room::Aspect aspect() const noexcept { return aspect_; }

	virtual const Part& part() const = 0;

	virtual std::string name() const = 0;

	bool leads_to(const Place& next) const override;
	bool within(const Abode& abode) const override;

protected:
	Room(Birth<Abode> birth, const Place& reflects, matter::Room kept);

	void host_only(const Contemplation& gaze) const;

private:
	const Abode& abode_;
	const Place& reflects_;
	const matter::Room::Aspect aspect_;
};


/// The window onto the words of a place: of its abode, «Слово» in the interface; of a tie,
/// «Ведение» or «Послушание». Whom its part admits enter it. Only its host writes here, and
/// the place it reflects decides what his words become; in the novice's cell of a tie, the
/// novice fulfils, trains and remarks on the exercises.
class Cell final : public Room {
public:
	Cell(Birth<Abode> birth, const Place& reflects, const Part& part, matter::Room kept);

	const Part& part() const override { return *part_.load(); }

	std::string name() const override;

	bool dwells(const Man& man) const override;

	void arrange(Birth<Abode> birth, const Part& part) const;

	const Place& source() const override;

	bool shows(const Man& who, const Word& word) const override;

	bool writable_by(const Man& writer) const;

	std::shared_ptr<const Word> write(const Contemplation& gaze, const Saying& saying) const;

	std::shared_ptr<const Training> train(const Contemplation& gaze, const Saying& title,
										  std::vector<Exercise> exercises) const;

	std::shared_ptr<const Deed> fulfil(const Contemplation& gaze, const Behest& behest, std::optional<Saying> report,
									  std::optional<std::vector<Exercise>> performed) const;
	void begin(const Contemplation& gaze, const Training& training, std::uint32_t exercise,
			   std::uint32_t approach) const;
	void end(const Contemplation& gaze, const Training& training) const;
	matter::Effort finish(const Contemplation& gaze, const Training& training, Weight weight,
						  std::uint32_t repetitions, Strain strain) const;
	matter::Remark remark(const Contemplation& gaze, const Training& training, std::uint32_t exercise,
						  const Saying& saying) const;

private:
	template<typename OtherTie, typename NotNovice>
	const Novice& obeying(const Contemplation& gaze, const Behest& behest) const;

	mutable std::atomic<const Part*> part_;
};


/// The threshold of an Abode, the core of its outer part and not inside it: anyone may
/// stand here, and from here one goes into the abode or on to other gates. One who does not
/// dwell behind them may knock, once while he stands here. Only the host keeps them, and
/// they are open while he stands in them: he lets in one who knocks as his acquaintance,
/// and bears a child of his here.
class Gates final : public Room, private Immanent<Heaven> {
public:
	Gates(Birth<Abode> birth, matter::Room kept);

	const Part& part() const override;

	std::string name() const override;

	bool dwells(const Man& man) const override;

	bool leads_to(const Place& next) const override;

	bool within(const Abode& abode) const override;

	bool keeps(const Man& man) const;

	bool open() const;

	void knock(const Contemplation& gaze) const;

	std::vector<std::reference_wrapper<const Man>> knocking() const;

	void admit(const Contemplation& gaze, const Man& man) const;

	const Man& bear(const Contemplation& gaze, const Unborn& unborn) const;
};


/// The host's own room, the core of the inner part, whence he steers his abode; no one else
/// enters. Here alone he answers those who ask him and asks himself, regards his dwellers
/// anew and moves his cells from part to part.
class Helm final : public Room {
public:
	Helm(Birth<Abode> birth, matter::Room kept);

	const Part& part() const override;

	std::string name() const override;

	bool dwells(const Man& man) const override;

	void regard(const Contemplation& gaze, const Man& dweller, matter::Dweller::Kind kind) const;

	void arrange(const Contemplation& gaze, const Cell& cell, const Part& part) const;

	void supplicate(const Contemplation& gaze, const Man& addressee) const;

	const Shepherding& accept(const Contemplation& gaze, const Supplication& ask) const;
	void reject(const Contemplation& gaze, const Supplication& ask) const;
};


} // namespace will::domain
