#pragma once

#include "matter/abode.h"
#include "properties/birth.h"
#include "matter/letter.h"
#include "places/place.h"
#include "identity/abode.h"
#include "values/abode_name.h"

#include <memory>
#include <mutex>
#include <unordered_set>
#include <vector>


namespace will::domain {


class Contemplation;
class Letter;
class Man;


/// Abode (Обитель) — a man's own place; others may dwell here.
/// Place id from Space::point; host keeps it. Rooms come later.
class Abode : public Place {
public:
	Abode(Birth<Man>, matter::Abode kept);

	/// Same value as Place::id().
	id::Abode abode_id() const noexcept { return id::Abode{id()}; }

	const AbodeName& name() const noexcept { return name_; }

	void admit(const Man& man);

	bool dwells(const Man& man) const override;

	/// A letter said here by the one whose gaze rests on it, born living from
	/// the matter the dimensions returned when they kept it; it enters the
	/// recollection of this abode.
	std::shared_ptr<const Letter> inscribe(const Contemplation& gaze, matter::Letter kept) const;

private:
	Abode(id::Abode id, AbodeName name);

	AbodeName name_;
	mutable std::unique_ptr<std::mutex> mutex_;
	std::unordered_set<const Man*> dwellers_;

};


} // namespace will::domain
