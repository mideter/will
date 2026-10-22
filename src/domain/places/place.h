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


/// Place (Место) — where a word is fixed: an abode, a tie, a room onto them. A place
/// reaches no dimension itself; Life recollects its words for the gazes upon it.
class Place : public Immanent<Space> {
public:
	virtual ~Place() = default;

	static const Place& of(id::Place id);

	id::Place id() const noexcept { return id_; }

	/// Only one who dwells in a place may contemplate it.
	virtual bool dwells(const Man& man) const = 0;

	/// The place whose words are seen here: itself, or what a room reflects.
	virtual const Place& source() const;

	/// All its words to all who dwell in it, unless it says otherwise.
	virtual bool shows(const Man& who, const Word& word) const;

	/// From here one turns only where the place leads: anywhere within an abode, and out
	/// of it or into another only through gates.
	virtual bool leads_to(const Place& next) const;

	/// Inside the abode, beyond its gates.
	virtual bool within(const Abode& abode) const;

	/// Asked by a cell of the place it reflects, of the cell's host, who alone writes there.
	/// An abode takes its host's words.
	virtual bool takes_words_of(const Man& host) const;

	/// The place has the cell's host do the deed its words are made by: record a letter in
	/// an abode, will a behest in a tie.
	virtual std::shared_ptr<const Word> receive(Birth<Cell> cell, const Saying& saying) const;

	std::shared_ptr<const Recollection> recollection(const Contemplation& gaze) const;

protected:
	/// The one held now, or a new one.
	std::shared_ptr<const Recollection> recollection() const;

	void enter(const Recollection& recollection, std::shared_ptr<const Word> word) const;

	/// For the virtual-base roles of a tie, which the living Tie supplies with Place(id).
	Place();
	explicit Place(id::Place id) noexcept;

private:
	id::Place id_;
};


} // namespace will::domain
