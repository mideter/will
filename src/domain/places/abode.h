#pragma once

#include "matter/abode.h"
#include "matter/letter.h"
#include "matter/room.h"
#include "places/helm.h"
#include "places/outer_abode.h"
#include "properties/birth.h"

#include <functional>
#include <memory>
#include <vector>


namespace will::domain {


class Letter;
class Man;
class Room;
class Supplication;
class Witness;
class World;


/// A man's own place: its outer, and within it the rooms set inward and the helm, which
/// only friends see. Its words are seen only through its rooms, and a dweller sees a room as
/// deep as his kind reaches; the host sees all. A new room is furnished inward: its own by
/// the host as the abode is born, one of a tie as the tie is bound. A kept room is recalled
/// where it is kept: its own by the host, one of a tie by the world once the tie lives. A
/// room moved goes whole from one side to the other; one place is reflected by one room. Its
/// rooms come outward first, each in the order they came.
class Abode final : public OuterAbode {
public:
	Abode(Birth<Man> birth, matter::Abode kept);
	~Abode() override;

	const Helm& helm() const noexcept { return helm_; }

	std::vector<std::reference_wrapper<const Room>> inner_rooms() const;

	std::vector<std::reference_wrapper<const Room>> rooms() const;

	const Room* room(const Place& reflects) const;

	bool inward(const Room& room) const;

	bool shows(const Man& man, const Room& room) const override;

	const Room& furnish(Birth<Man> host, const matter::Room& kept) const;
	const Room& furnish(Birth<Supplication> supplication, const matter::Room& kept) const;

	void recall(Birth<Man> host, const matter::Room& kept) const;
	void recall(Birth<World> world, const matter::Room& kept) const;

	void arrange(Birth<Man> host, const matter::Room& kept) const;

	std::shared_ptr<const Word> receive(Birth<Room> room, const Saying& saying) const override;

	std::shared_ptr<const Letter> inscribe(Birth<Witness>, matter::Letter kept) const;

private:
	const Room& lay(const matter::Room& kept) const;

	Helm helm_;
	mutable std::vector<std::unique_ptr<Room>> inner_rooms_;
};


} // namespace will::domain
