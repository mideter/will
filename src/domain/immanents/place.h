#pragma once

#include "identity/place.h"
#include "matter/deed.h"
#include "matter/letter.h"
#include "properties/immanent.h"

#include <memory>
#include <vector>


namespace will::domain {


class Life;
class Space;
class Word;


/// Place (Место) — where a Word may be fixed in time; immanent to Space.
/// Abode is a place; a living Obedience/Shepherding pair is Tie (Узы).
/// Living places are known to Space; Place::of looks them up.
/// A place reaches no dimension by itself: Life gives it the matter of its
/// words and remembers which of them live now.
class Place : public Immanent<Space> {
public:
	virtual ~Place() = default;

	/// Throws if unknown.
	static const Place& of(id::Place id);

	id::Place id() const noexcept { return id_; }

protected:
	/// The matter of the words placed here, oldest first, as Life gives it.
	std::vector<matter::Letter> kept_letters() const;
	std::vector<matter::Deed> kept_deeds() const;

	/// The words placed here that live now, as Life remembers them.
	std::vector<std::shared_ptr<const Word>> living_words(bool& whole) const;
	void remember(const std::vector<std::shared_ptr<const Word>>& words) const;
	void remember(const std::shared_ptr<const Word>& word) const;

	/// For virtual-base roles (Obedience/Shepherding) that are never most-derived;
	/// the living Tie supplies Place(id). Throws if actually invoked.
	Place();
	explicit Place(id::Place id) noexcept;

private:
	id::Place id_;
};


} // namespace will::domain
