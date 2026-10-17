#pragma once

#include "matter/supplication.h"
#include "matter/training.h"

#include "relations/contemplation.h"
#include "matter/word.h"
#include "horizons/heaven.h"
#include "identity/word.h"
#include "dimensions/eternity.h"
#include "properties/immanent.h"
#include "values/saying.h"

#include <memory>
#include <vector>


namespace will::domain {


class Place;
class Soul;
class Supplication;


/// Spirit (Дух) — the highest of the soul: it alone utters.
class Spirit : public Immanent<Heaven> {
public:
	virtual ~Spirit() = default;

protected:
	Spirit() = default;

	matter::Word utter(const Saying& saying) const;

	matter::Training exercise(id::Word word, std::vector<Exercise> exercises) const;

	void contemplate(const Place& place) const;

	void repose() const;

	/// Heaven keeps it until it is answered.
	static void entrust(matter::Supplication kept);

	std::shared_ptr<const Contemplation> contemplation() const;

private:
	static Eternity& eternity() { return horizon().eternity(); }
};


} // namespace will::domain
