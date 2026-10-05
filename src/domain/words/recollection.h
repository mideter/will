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


/// Recollection (Воспоминание) — the words of one place called to life from
/// the memory the dimensions keep. The gazes upon the place hold it; while it
/// is held, all its words live, and they end with it: the words of a place live
/// all or none. Memory stays kept. Born of Life, which knows it while it is
/// held; immanent to Life. A word newly placed enters it, brought by its place.
class Recollection : public Immanent<Life> {
public:
	Recollection(Birth<Life>, const Place& place, std::vector<std::shared_ptr<const Word>> words);

	const Place& place() const noexcept { return place_; }

	/// Its words, oldest first.
	std::vector<std::shared_ptr<const Word>> words() const;

	/// A word newly placed enters, brought by the place it is placed in.
	/// Throws if another place brings it.
	void enter(Birth<Place> birth, std::shared_ptr<const Word> word) const;

private:
	const Place& place_;

	mutable std::mutex mutex_;
	mutable std::vector<std::shared_ptr<const Word>> words_;
};


} // namespace will::domain
