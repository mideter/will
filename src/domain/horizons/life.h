#pragma once

#include "identity/place.h"
#include "identity/word.h"
#include "matter/remark.h"
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


/// Proceeds from Eternity and brings forth the World in its one Creation; it recollects the
/// words of a place from memory while the gazes hold them, one recollection at a time,
/// oldest first.
class Life : private Immanent<Eternity> {
public:
	Life();
	~Life();

	Eternity& eternity() const;

	Creation& create();

	Creation& creation() const;

	static std::shared_ptr<const Recollection> recollection(Birth<Place> birth);

private:
	static Life& the();

	Spatiality& spatiality() const;
	Temporality& temporality() const;

	std::shared_ptr<const Recollection> recollection(const Place& place) const;

	std::vector<std::shared_ptr<const Word>> recall(const Place& place) const;

	std::unordered_map<id::Word, std::vector<matter::Remark>> remarks_on(const std::vector<id::Word>& trainings) const;

	static Life* current_;

	std::unique_ptr<Creation> creation_;

	mutable std::mutex mutex_;
	mutable std::unordered_map<id::Place, std::weak_ptr<const Recollection>> recollections_;
};


} // namespace will::domain
