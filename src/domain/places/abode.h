#pragma once

#include "matter/abode.h"
#include "matter/dweller.h"
#include "matter/room.h"
#include "properties/birth.h"
#include "matter/letter.h"
#include "places/place.h"
#include "values/abode_name.h"

#include <memory>
#include <string_view>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>


namespace will::domain {


class Acquaintance;
class Behest;
class Contemplation;
class Letter;
class Man;
class Gates;
class UpperRoom;
class Room;
class Supplication;
class World;


/// Abode (Обитель) — a man's own place, born of its host; in a sense the host
/// is his Abode. Its words are seen only through its rooms: the cell reflects
/// the abode itself, a tie room a tie. Others dwell here as the host admits
/// them: each is born an Acquaintance, a Neighbour or a Friend, and enters the
/// rooms of the part his kind opens. Looking at the abode itself, the host sees
/// what awaits him in its rooms; one writes only in a room.
class Abode : public Place {
public:
	Abode(Birth<Man> birth, matter::Abode kept);
	~Abode() override;

	const AbodeName& name() const noexcept { return name_; }

	const Man& host() const noexcept { return host_; }

	/// A man dwells here as his dweller matter says: the host admits him or
	/// regards him anew, or the World recalls him on awakening. Throws if the
	/// matter is of another abode or another soul, or the man is the host.
	void admit(Birth<Man> birth, const Man& man, const matter::Dweller& kept);
	void admit(Birth<World> birth, const Man& man, const matter::Dweller& kept);

	/// The dwellers of this abode, in no particular order.
	std::vector<std::shared_ptr<const Acquaintance>> dwellers() const;

	/// A room is born here from its matter: the host furnishes it, or the World
	/// recalls it on awakening. Throws if the matter is of another abode or the
	/// abode already has a room reflecting that place.
	const Room& furnish(Birth<Man> birth, const matter::Room& kept);
	const Room& furnish(Birth<World> birth, const matter::Room& kept);
	const Room& furnish(Birth<Supplication> birth, const matter::Room& kept);

	/// A room of this abode stands now where the host arranged it.
	void arrange(Birth<Man> birth, const matter::Room& kept) const;

	/// The rooms of this abode, in the order they were furnished.
	std::vector<std::reference_wrapper<const Room>> rooms() const;

	/// The room of this abode reflecting the words of this place; none if there is none.
	const Room* room(const Place& reflects) const;

	/// The standard rooms: the gates and the upper room. Every abode has them.
	const Gates& gates() const;
	const UpperRoom& upper_room() const;

	/// The room of this abode by its name; none if there is none.
	const Room* room(std::string_view name) const;

	/// This man as a dweller here; none for the host and for a stranger.
	std::shared_ptr<const Acquaintance> dweller(const Man& man) const;

	/// The host and his dwellers dwell here.
	bool dwells(const Man& man) const override;

	/// The abode itself shows no words; they are seen through its rooms.
	bool shows(const Man& who, const Word& word) const override;

	/// What awaits the host in the tie rooms: behests not yet fulfilled, oldest
	/// first by room.
	std::vector<std::shared_ptr<const Behest>> outstanding() const;

	/// A letter said here by its host looking through the cell, born living
	/// from the matter the dimensions returned when they kept it; it enters the
	/// recollection of this abode.
	std::shared_ptr<const Letter> inscribe(const Contemplation& gaze, matter::Letter kept) const;

private:
	void admit(const Man& man, const matter::Dweller& kept);
	const Room& furnish(const matter::Room& kept);

	const Man& host_;
	AbodeName name_;
	mutable std::unique_ptr<std::mutex> mutex_;
	std::unordered_map<const Man*, std::shared_ptr<const Acquaintance>> dwellers_;
	std::vector<std::unique_ptr<Room>> rooms_;

};


} // namespace will::domain
