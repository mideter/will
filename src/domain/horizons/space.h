#pragma once

#include "properties/immanent.h"
#include "identity/place.h"

#include <cstdint>
#include <mutex>
#include <unordered_map>


namespace will::domain {


class Place;


/// The living field of places, given through Eternity as Time is.
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

	/// Without persisting it.
	void seed(std::uint64_t value);

	/// Durable implementation may keep the mark.
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
