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
#include <string_view>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>


namespace will::domain {


class Acquaintance;
class Behest;
class Letter;
class Man;
class Gates;
class Helm;
class Witness;
class Cell;
class Room;
class Supplication;
class World;


/// A man's own place: its words are seen only through its rooms, and others dwell in it as
/// its host lets them in at the gates. Every abode has its gates and its helm; its rooms
/// keep the order they were furnished in. In it dwell the host, his dwellers and his fathers
/// by spirit, who are ever friends yet unseen among the dwellers. What it owes and awaits
/// are the behests not yet fulfilled where its host is the novice or the testator; what it
/// has to review, the behests fulfilled where he is the testator, whose deeds he has not yet
/// seen; each oldest first by room.
class Abode : public Place {
public:
	Abode(Birth<Man> birth, matter::Abode kept);
	~Abode() override;

	const AbodeName& name() const noexcept { return name_; }

	const Outer& outer() const noexcept { return outer_; }
	const Inner& inner() const noexcept { return inner_; }

	const Man& host() const noexcept { return host_; }

	void admit(Birth<Man> birth, const Man& man, const matter::Dweller& kept);
	void admit(Birth<World> birth, const Man& man, const matter::Dweller& kept);

	void admit(Birth<Gates> birth, const Man& man, const matter::Dweller& kept);

	std::vector<std::shared_ptr<const Acquaintance>> dwellers() const;

	const Room& furnish(Birth<Man> birth, const matter::Room& kept);
	const Room& furnish(Birth<World> birth, const matter::Room& kept);
	const Room& furnish(Birth<Supplication> birth, const matter::Room& kept);

	void arrange(Birth<Man> birth, const matter::Room& kept) const;

	std::vector<std::reference_wrapper<const Room>> rooms() const;

	const Cell* cell(const Place& reflects) const;
	const Cell* cell(std::string_view name) const;

	const Gates& gates() const;
	const Helm& helm() const;

	const Room* room(std::string_view name) const;

	std::shared_ptr<const Acquaintance> dweller(const Man& man) const;

	bool dwells(const Man& man) const override;

	bool shows(const Man& who, const Word& word) const override;

	bool leads_to(const Place& next) const override;
	bool within(const Abode& abode) const override;

	std::shared_ptr<const Word> receive(Birth<Cell> cell, const Saying& saying) const override;

	std::vector<std::shared_ptr<const Behest>> owed() const;

	std::vector<std::shared_ptr<const Behest>> awaited() const;

	std::vector<std::shared_ptr<const Behest>> unreviewed() const;

	std::shared_ptr<const Letter> inscribe(Birth<Witness>, matter::Letter kept) const;

	bool ancestor(const Man& man) const;

private:
	std::vector<std::shared_ptr<const Behest>> unfulfilled(bool host_is_novice) const;

	void admit(const Man& man, const matter::Dweller& kept);
	const Room& furnish(const matter::Room& kept);
	const Part& part(matter::Room::Part kept) const;

	const Man& host_;
	AbodeName name_;
	Outer outer_;
	Inner inner_;
	mutable std::unique_ptr<std::mutex> mutex_;
	std::unordered_map<const Man*, std::shared_ptr<const Acquaintance>> dwellers_;
	std::vector<std::unique_ptr<Room>> rooms_;

};


} // namespace will::domain
