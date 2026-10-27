#pragma once

#include "matter/abode.h"
#include "matter/dweller.h"
#include "matter/room.h"
#include "properties/birth.h"
#include "matter/letter.h"
#include "places/part.h"
#include "places/place.h"
#include "values/abode_name.h"

#include <memory>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>


namespace will::domain {


class Acquaintance;
class Room;
class Contemplation;
class Letter;
class Man;
class Witness;


/// A man's own place, of two parts, the outer and the inner: its words are seen only
/// through its rooms, and others dwell in it as its host brings them, each born anew of his
/// kind. Only its host acts in the places within it; its rooms come by its parts, the outer
/// first, each in the order they came into it. In it dwell the host, his dwellers and his
/// fathers by spirit, who are ever friends yet unseen among the dwellers.
class Abode : public Place {
public:
	Abode(Birth<Man> birth, matter::Abode kept);
	~Abode() override;

	const AbodeName& name() const noexcept { return name_; }

	const Outer& outer() const noexcept { return outer_; }
	const Inner& inner() const noexcept { return inner_; }

	const Man& host() const noexcept { return host_; }

	void admit(Birth<Man> host, const matter::Dweller& kept);

	std::vector<std::shared_ptr<const Acquaintance>> dwellers() const;

	const Part& part(matter::Room::Part kept) const;

	void arrange(Birth<Man> birth, const matter::Room& kept) const;

	std::vector<std::reference_wrapper<const Room>> rooms() const;

	const Room* room(const Place& reflects) const;

	void host_only(const Contemplation& gaze, const Place& here) const;

	std::shared_ptr<const Acquaintance> dweller(const Man& man) const;

	bool dwells(const Man& man) const override;

	bool leads_to(const Man& who, const Place& next) const override;
	bool within(const Abode& abode) const override;

	std::shared_ptr<const Word> receive(Birth<Room> room, const Saying& saying) const override;

	std::shared_ptr<const Letter> inscribe(Birth<Witness>, matter::Letter kept) const;

private:
	const Man& host_;
	AbodeName name_;
	Outer outer_;
	Inner inner_;
	mutable std::unique_ptr<std::mutex> mutex_;
	std::unordered_map<const Man*, std::shared_ptr<const Acquaintance>> dwellers_;
};


} // namespace will::domain
