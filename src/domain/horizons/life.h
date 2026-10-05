#pragma once

#include "identity/place.h"
#include "properties/birth.h"
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
/// Words are immanent to Life: it recollects the words of a place from memory,
/// bringing forth their Recollection, and knows it while the gazes hold it.
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

	/// The recollection of the place that asks: the one held now, or, if none
	/// is, a new one that Life recollects.
	static std::shared_ptr<const Recollection> recollection(Birth<Place> birth);

private:
	static Life& the();

	/// Of Life's Creation; throw if Life has not created yet.
	Spatiality& spatiality() const;
	Temporality& temporality() const;

	std::shared_ptr<const Recollection> recollection(const Place& place) const;

	/// The words placed in this place, oldest first (capped by
	/// Spatiality::MaxLetterLimit), born from the memory the dimensions keep: in
	/// an abode as letters; in a tie as behests and the deeds that fulfil them.
	std::vector<std::shared_ptr<const Word>> recall(const Place& place) const;

	static Life* current_;

	std::unique_ptr<Creation> creation_;

	mutable std::mutex mutex_;
	mutable std::unordered_map<id::Place, std::weak_ptr<const Recollection>> recollections_;
};


} // namespace will::domain
