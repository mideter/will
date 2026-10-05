#pragma once

#include "identity/place.h"
#include "properties/birth.h"
#include "properties/immanent.h"

#include <memory>
#include <vector>


namespace will::domain {


class Contemplation;
class Life;
class Man;
class Space;
class Life;
class Recollection;
class Word;


/// Place (Место) — where a Word may be fixed in time; immanent to Space.
/// Abode is a place; a living Obedience/Shepherding pair is Tie (Узы).
/// Living places are known to Space; Place::of looks them up.
/// A place reaches no dimension by itself: Life recollects its words from
/// memory as their Recollection, which the gazes upon it hold.
class Place : public Immanent<Space> {
public:
	virtual ~Place() = default;

	/// Throws if unknown.
	static const Place& of(id::Place id);

	id::Place id() const noexcept { return id_; }

	/// Whether this man dwells here: only one who dwells in a place may contemplate it.
	virtual bool dwells(const Man& man) const = 0;

	/// Whether this place shows this word of its own to the one who looks; a
	/// place shows all its words to all who dwell in it, unless it says otherwise.
	virtual bool shows(const Man& who, const Word& word) const;

	/// The recollection of this place, for a gaze upon it. Throws if the gaze
	/// rests elsewhere.
	std::shared_ptr<const Recollection> recollection(const Contemplation& gaze) const;

protected:
	/// The recollection of this place: the one held now, or a new one.
	std::shared_ptr<const Recollection> recollection() const;

	/// A word newly placed here enters the recollection of this place.
	void enter(const Recollection& recollection, std::shared_ptr<const Word> word) const;

	/// For virtual-base roles (Obedience/Shepherding) that are never most-derived;
	/// the living Tie supplies Place(id). Throws if actually invoked.
	Place();
	explicit Place(id::Place id) noexcept;

private:
	id::Place id_;
};


} // namespace will::domain
