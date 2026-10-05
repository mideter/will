#pragma once

#include "identity/place.h"
#include "matter/behest.h"
#include "matter/deed.h"
#include "matter/letter.h"
#include "properties/immanent.h"

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>


namespace will::domain {


class Creation;
class Place;
class Eternity;
class Spatiality;
class Temporality;
class Word;


/// Life (Жизнь) — proceeds from Eternity, is before the World and brings it
/// forth: Creation is the act of Life, and nothing else creates. Life outlives
/// the World it made. It knows the Creation present in it and reaches
/// Spatiality and Temporality through it. One Life; immanent to Eternity.
/// Words are immanent to Life: it gives places the matter of their words and
/// remembers which words live now, without owning them — the gazes hold them.
class Life : private Immanent<Eternity> {
public:
	/// Throws if no Eternity is realised.
	Life();
	~Life();

	/// Bring forth the World from Eternity and awaken it.
	Creation create();

	/// The words of a place living now. Whole when Life remembers them all alive.
	struct Remembered {
		std::vector<std::shared_ptr<const Word>> words;
		bool whole = false;
	};

private:
	friend class Place;
	friend class Immanent<Life>;

	static Life& the();

	/// Throws if a Creation is already present.
	void present(const Creation& creation);
	void depart(const Creation& creation);

	/// Of the present Creation; throws if there is none.
	Spatiality& spatiality() const;
	Temporality& temporality() const;

	/// The matter of the words placed in this place, oldest first (capped by
	/// Spatiality::MaxLetterLimit): as letters; or, in a tie, as the behests and
	/// the deeds that fulfil them.
	std::vector<matter::Letter> letters(id::Place place) const;
	std::vector<matter::Behest> behests(id::Place place) const;
	std::vector<matter::Deed> deeds(id::Place place) const;

	/// Executions naming any of these words as their deed, by deed.
	std::unordered_map<id::Word, matter::Execution> executions_of(const std::vector<matter::Letter>& parts) const;

	Remembered remembered(id::Place place) const;
	void remember(id::Place place, const std::vector<std::shared_ptr<const Word>>& words);
	void remember(id::Place place, const std::shared_ptr<const Word>& word);

	static Life* current_;

	const Creation* creation_ = nullptr;

	mutable std::mutex mutex_;
	std::unordered_map<id::Place, std::vector<std::weak_ptr<const Word>>> living_;
};


} // namespace will::domain
