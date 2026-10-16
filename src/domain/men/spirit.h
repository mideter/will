#pragma once

#include "matter/training.h"

#include "relations/contemplation.h"
#include "matter/word.h"
#include "horizons/heaven.h"
#include "identity/word.h"
#include "identity/soul.h"
#include "dimensions/eternity.h"
#include "properties/immanent.h"
#include "values/saying.h"
#include "values/timestamp.h"

#include <memory>
#include <vector>


namespace will::domain {


class Abode;
class Place;
class Soul;
class Supplication;


/// Spirit (Дух) — the highest of the soul: it alone utters.
class Spirit : public Immanent<Heaven> {
public:
	virtual ~Spirit() = default;

protected:
	Spirit() = default;

	static const Soul& soul(id::Soul id);

	matter::Word utter(const Saying& saying) const;

	Timestamp now() const;

	matter::Training exercise(id::Word word, std::vector<Exercise> exercises) const;

	bool knows(id::Soul soul_id) const;

	void contemplate(const Place& place) const;

	void cease() const;

	/// Oldest first.
	std::vector<std::shared_ptr<const Supplication>> supplications() const;

	static void keep(std::shared_ptr<const Supplication> supplication);

	static void release(const Supplication& supplication);

	std::shared_ptr<const Contemplation> contemplation() const;

private:
	static Heaven& heaven() { return Heaven::the(); }

	static Eternity& eternity() { return heaven().eternity(); }
};


} // namespace will::domain
