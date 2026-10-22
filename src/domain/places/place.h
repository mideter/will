#pragma once

#include "identity/place.h"
#include "properties/birth.h"
#include "properties/immanent.h"

#include <memory>
#include <optional>
#include <vector>


namespace will::domain {


class Abode;
class Cell;
class Contemplation;
class Man;
class Space;
class Recollection;
class Saying;
class Word;


/// Where a word is fixed: an abode, a tie, a room onto them. A place reaches no dimension
/// itself; Life recollects its words for the gazes upon it, one recollection while it is
/// beheld. Only one who dwells in a place may contemplate it, and it shows all its words to
/// all who dwell in it unless it says otherwise. From here one turns only where the place
/// leads: anywhere within an abode, and out of it or into another only through gates. A
/// cell asks the place it reflects whether it takes its host's words, and has him do the
/// deed they are made by: an abode takes a letter, a tie a behest. The roles of a tie share
/// it as a virtual base, given its id by the living Tie.
class Place : public Immanent<Space> {
public:
	virtual ~Place() = default;

	static const Place& of(id::Place id);

	id::Place id() const noexcept { return id_; }

	virtual bool dwells(const Man& man) const = 0;

	virtual const Place& source() const;

	virtual bool shows(const Man& who, const Word& word) const;

	virtual bool leads_to(const Place& next) const;

	virtual bool within(const Abode& abode) const;

	virtual bool takes_words_of(const Man& host) const;

	virtual std::shared_ptr<const Word> receive(Birth<Cell> cell, const Saying& saying) const;

	std::shared_ptr<const Recollection> recollection(const Contemplation& gaze) const;

protected:
	std::shared_ptr<const Recollection> recollection() const;

	void enter(const Recollection& recollection, std::shared_ptr<const Word> word) const;

	Place();
	explicit Place(id::Place id) noexcept;

private:
	id::Place id_;
};


} // namespace will::domain
