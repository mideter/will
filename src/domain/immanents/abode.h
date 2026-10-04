#pragma once

#include "matter/abode.h"
#include "matter/letter.h"
#include "immanents/place.h"
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
	Abode(id::Abode id, AbodeName name);
	explicit Abode(matter::Abode kept);

	/// Same value as Place::id().
	id::Abode abode_id() const noexcept { return id::Abode{id()}; }

	const AbodeName& name() const noexcept { return name_; }

	void admit(const Man& man);

	bool dwells(const Man& man) const;

	/// Letters placed here, oldest first, for a gaze upon this abode (only one
	/// who dwells here may contemplate it). The ones still living are shared;
	/// the rest are born from what the dimensions keep, read through the gazer.
	std::vector<std::shared_ptr<const Letter>> letters(const Contemplation& gaze) const;

	/// A letter said here by the one whose gaze rests on it, born living from
	/// the matter the dimensions returned when they kept it.
	std::shared_ptr<const Letter> inscribe(const Contemplation& gaze, matter::Letter kept) const;

private:
	AbodeName name_;
	mutable std::unique_ptr<std::mutex> mutex_;
	std::unordered_set<const Man*> dwellers_;

	/// The letters living now, remembered but not owned: the gazes own them.
	mutable std::vector<std::weak_ptr<const Letter>> living_;
};


} // namespace will::domain
