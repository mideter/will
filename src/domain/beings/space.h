#pragma once

#include "identity/place.h"

#include <mutex>
#include <unordered_map>


namespace will::domain {


class Place;
class Creation;
class Spatiality;


/// Space (Пространство) — living field of Places in the World.
/// Heaven and Earth are in space and time; Space knows places by id (non-owning).
/// Creation brings forth Space. Place reaches it via Place::of / present().
/// Space points place ids (as Time gives instants); Spatiality remembers the mark.
class Space {
public:
	/// Point a new place — persistent id, like Time::instant for the when.
	static id::Place point();

	/// Whether Space knows this place.
	bool knows(id::Place id) const;

	/// Living place in the waking cosmos. Throws if unknown.
	const Place& place(id::Place id) const;

protected:
	friend class Creation;
	friend class Place;

	/// Bring forth Space (Creation); Spatiality holds the place-id mark.
	explicit Space(Spatiality& spatiality);

	~Space();

	Space(const Space&) = delete;
	Space& operator=(const Space&) = delete;
	Space(Space&& other) noexcept;
	Space& operator=(Space&&) = delete;

	/// Present a place owned on the heap (Abode, Tie).
	void present(const Place& place);

private:
	/// The one living Space. Throws if not yet brought forth / already destroyed.
	static Space& the();

	static Space* current_;

	Spatiality& spatiality_;
	mutable std::mutex mutex_;
	std::unordered_map<id::Place, const Place*> places_;
};


} // namespace will::domain
