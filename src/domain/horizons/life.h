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
class Recollection;
class Eternity;
class Spatiality;
class Temporality;
class Word;


/// Life (Жизнь) — proceeds from Eternity, is before the World and brings it
/// forth: Creation is the act of Life, and nothing else creates. Life outlives
/// the World it made. Life holds its one Creation and reaches Spatiality and
/// Temporality through it. One Life; immanent to Eternity.
/// Words are immanent to Life: it gives places the matter of their words and
/// brings forth their Recollection, knowing it while the gazes hold it.
class Life : private Immanent<Eternity> {
public:
	/// Throws if no Eternity is realised.
	Life();
	~Life();

	/// Eternity Life proceeds from.
	Eternity& eternity() const;

	/// Bring forth the World from Eternity and awaken it. Throws if Life has
	/// already created: there is one Creation in Life.
	Creation& create();

	/// Throws if Life has not created yet.
	Creation& creation() const;

private:
	friend class Place;

	static Life& the();

	/// Of Life's Creation; throw if Life has not created yet.
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

	/// The recollection of this place held now, or, if none is, a new one of
	/// the words the place recalls.
	std::shared_ptr<const Recollection> recollection(const Place& place) const;

	static Life* current_;

	std::unique_ptr<Creation> creation_;

	mutable std::mutex mutex_;
	mutable std::unordered_map<id::Place, std::weak_ptr<const Recollection>> recollections_;
};


} // namespace will::domain
