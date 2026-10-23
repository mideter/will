#pragma once

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


/// The highest of the soul, its eye: it alone utters, beholds and asks; Heaven keeps what it
/// asks until it is answered.
class Spirit : public Immanent<Heaven> {
public:
	virtual ~Spirit() = default;

protected:
	Spirit() = default;

	matter::Word utter(const Saying& saying) const;

	matter::Training utter(id::Word word, std::vector<Exercise> exercises) const;

	matter::Remarked remark(id::Word remark, id::Word training, std::uint32_t exercise) const;

	void contemplate(const Place& place) const;

	void repose() const;

	void ask(const Soul& addressee) const;

	std::shared_ptr<const Contemplation> contemplation() const;

private:
	static Eternity& eternity() { return horizon().eternity(); }
};


} // namespace will::domain
