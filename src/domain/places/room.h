#pragma once

#include "matter/dweller.h"
#include "matter/room.h"
#include "places/place.h"
#include "properties/birth.h"
#include "properties/immanent.h"

#include "matter/effort.h"
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
class Training;


/// Room (Комната) — a window in an Abode onto a place, with no words of its own.
class Room : public Place {
public:
	const Abode& abode() const noexcept { return abode_; }
	const Place& reflects() const noexcept { return reflects_; }
	matter::Room::Aspect aspect() const noexcept { return aspect_; }
	matter::Room::Part part() const noexcept { return part_.load(); }

	virtual std::string name() const = 0;

	void arrange(Birth<Abode> birth, const matter::Room& kept) const;

	/// The host, and the dwellers whose kind enters its part.
	bool dwells(const Man& man) const override;

protected:
	Room(Birth<Abode> birth, const Place& reflects, matter::Room kept);

private:
	const Abode& abode_;
	const Place& reflects_;
	const matter::Room::Aspect aspect_;
	mutable std::atomic<matter::Room::Part> part_;
};


/// Cell (Келья) — the window onto the words of a place: of its abode, «Слово» in the
/// interface; of a tie, «Ведение» or «Послушание».
class Cell final : public Room {
public:
	Cell(Birth<Abode> birth, const Place& reflects, matter::Room kept);

	std::string name() const override;

	const Place& source() const override;

	bool shows(const Man& who, const Word& word) const override;

	/// Whoever stands here writes as its host, what the place it reflects takes of him.
	bool writable() const;

	/// What the words become, the place it reflects decides.
	std::shared_ptr<const Word> write(const Contemplation& gaze, const Saying& saying) const;

	std::shared_ptr<const Training> train(const Contemplation& gaze, const Saying& title,
										  std::vector<Exercise> exercises) const;

	/// In the novice's cell of a tie, whoever stands there does what the novice does.
	std::shared_ptr<const Deed> fulfil(const Contemplation& gaze, const Behest& behest, std::optional<Saying> report,
									  std::optional<std::vector<Exercise>> performed) const;
	void begin(const Contemplation& gaze, const Training& training, std::uint32_t exercise,
			   std::uint32_t approach) const;
	matter::Effort finish(const Contemplation& gaze, const Training& training, Weight weight,
						  std::uint32_t repetitions) const;

private:
	template<typename OtherTie, typename NotNovice>
	const Novice& obeying(const Contemplation& gaze, const Behest& behest) const;
};


/// Gates (Врата) — the threshold of an Abode, through which one comes in, born or to be
/// born: those who keep them let in an acquaintance of the host and bear a child of
/// his, whoever does it.
class Gates final : public Room, private Immanent<Heaven> {
public:
	Gates(Birth<Abode> birth, matter::Room kept);

	std::string name() const override;

	/// Anyone may stand at the gates.
	bool dwells(const Man& man) const override;

	/// The host, or a dweller whose kind enters the part they stand in.
	bool keeps(const Man& man) const;

	/// Open while one who keeps them stands in them.
	bool open() const;

	/// The one let in becomes the host's acquaintance, whoever keeps the gates.
	void admit(const Contemplation& gaze, const Man& man) const;
};


/// UpperRoom (Горница) — the room of an Abode's dwellers: here alone the host regards
/// them anew and arranges his rooms.
class UpperRoom final : public Room {
public:
	UpperRoom(Birth<Abode> birth, matter::Room kept);

	std::string name() const override;

	/// Whoever stands here regards as the host, but not himself.
	void regard(const Contemplation& gaze, const Man& dweller, matter::Dweller::Kind kind) const;

	void arrange(const Contemplation& gaze, const Room& room, matter::Room::Part part) const;

	/// Only its host asks one of its dwellers to become his testator.
	void supplicate(const Contemplation& gaze, const Man& addressee) const;
};


} // namespace will::domain
