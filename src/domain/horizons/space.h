#pragma once

#include "properties/immanent.h"
#include "identity/place.h"

#include <cstdint>
#include <mutex>
#include <unordered_map>


namespace will::domain {


class Place;


/// The living field of places, given through Eternity as Time is. It marks each point it
/// gives; a durable space keeps the mark and resumes from it without marking anew.
class Space : private Immanent<Space> {
public:
	Space();
	virtual ~Space();

	id::Place point();

	bool knows(id::Place id) const;

	const Place& place(id::Place id) const;

protected:
	friend class Place;
	friend class Immanent<Space>;

	void resume_from_mark(std::uint64_t value);

	virtual void persist_mark(id::Place id) {}

private:
	void present(const Place& place);

	static Space& the();

	static Space* current_;

	std::uint64_t high_water_ = 0;
	mutable std::mutex mutex_;
	std::unordered_map<id::Place, const Place*> places_;
};


} // namespace will::domain
