#pragma once

#include "properties/birth.h"
#include "properties/immanent.h"

#include <memory>
#include <mutex>
#include <vector>


namespace will::domain {


class Life;
class Place;
class Word;


/// Recollection (Воспоминание) — the words of a place called to life from memory while
/// the gazes hold it: they live all or none.
class Recollection : public Immanent<Life> {
public:
	Recollection(Birth<Life>, const Place& place, std::vector<std::shared_ptr<const Word>> words);

	const Place& place() const noexcept { return place_; }

	/// Oldest first.
	std::vector<std::shared_ptr<const Word>> words() const;

	void enter(Birth<Place> birth, std::shared_ptr<const Word> word) const;

private:
	const Place& place_;

	mutable std::mutex mutex_;
	mutable std::vector<std::shared_ptr<const Word>> words_;
};


} // namespace will::domain
