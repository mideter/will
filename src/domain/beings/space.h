#pragma once

#include "properties/immanent.h"
#include "identity/place.h"

#include <cstdint>
#include <mutex>
#include <unordered_map>


namespace will::domain {


class Place;


/// Space (Пространство) — living field of Places in the World.
/// One Space, given through Eternity (as Time is). Place reaches it via Place::of / present().
/// Points place ids as Time gives instants.
class Space : private Immanent {
public:
	Space();
	virtual ~Space();

	/// Point a new place — persistent id, like Time::instant for the when.
	id::Place point();

	/// Whether Space knows this place.
	bool knows(id::Place id) const;

	/// Living place in the waking cosmos. Throws if unknown.
	const Place& place(id::Place id) const;

protected:
	friend class Place;

	/// Present a place owned on the heap (Abode, Tie).
	void present(const Place& place);

	/// Load a durable mark without persisting (Eternity impl at bring-forth).
	void seed(std::uint64_t value);

	/// Durable face may remember the mark.
	virtual void persist_mark(id::Place id) {}

private:
	/// The one living Space. Throws if not yet brought forth / already destroyed.
	static Space& the();

	static Space* current_;

	std::uint64_t high_water_ = 0;
	mutable std::mutex mutex_;
	std::unordered_map<id::Place, const Place*> places_;
};


} // namespace will::domain
