#pragma once

#include "identity/place.h"
#include "properties/birth.h"
#include "properties/immanent.h"
#include "values/refusal.h"

#include <memory>
#include <optional>
#include <vector>


namespace will::domain {


class Contemplation;
class Life;
class Man;
class Space;
class Life;
class Recollection;
class Saying;
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

	/// The place whose words are seen here: the place itself, or, for a room,
	/// the place it reflects.
	virtual const Place& source() const;

	/// Whether this place shows this word to the one who looks; a place shows
	/// all its words to all who dwell in it, unless it says otherwise.
	virtual bool shows(const Man& who, const Word& word) const;

	/// Whether one standing here may write, and if not, why. One writes in a room
	/// that shows the words of one's own abode or of a tie, and only as the place
	/// whose words they are takes them; anywhere else the world refuses.
	virtual std::optional<Refusal> refuses_word_of(const Man& writer) const;

	/// Whose words this place takes as written here, asked of the place whose words
	/// they are: the host's in an abode, the testator's in a tie; none elsewhere.
	virtual std::optional<Refusal> refuses_writer(const Man& writer) const;

	/// What one writes here becomes, by this place: the place whose words they are
	/// has the writer do the deed its words are made by — record a letter in an
	/// abode, will a behest in a tie. Asked once it has not refused the writer.
	virtual std::shared_ptr<const Word> receive(const Contemplation& gaze, const Saying& saying) const;

	/// The recollection of the words seen here, for a gaze upon them. Throws if
	/// the gaze sees other words.
	std::shared_ptr<const Recollection> recollection(const Contemplation& gaze) const;

protected:
	/// The recollection of the words seen here: the one held now, or a new one.
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
