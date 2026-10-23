#pragma once

#include "matter/fatherhood.h"
#include "matter/remark.h"
#include "matter/soul.h"
#include "matter/training.h"
#include "matter/word.h"
#include "identity/word.h"
#include "identity/soul.h"
#include "horizons/time.h"
#include "properties/birth.h"
#include "properties/immanent.h"
#include "values/soul_name.h"
#include "values/saying.h"

#include <memory>
#include <vector>


namespace will::domain {


class Creation;
class Space;
class Spatiality;
class Temporality;


/// What endures: souls, their words, fatherhood. Time and Space are reached only from it;
/// it realises Spatiality and Temporality for a Creation. A child has one father by flesh;
/// a father by spirit chosen anew replaces the former. A word has its exercises once, and is
/// a remark on an exercise once.
/// Words asked for come in the order asked, the unknown passed over; one word asked for
/// alone must be known. Reached only while it is realised.
class Eternity {
public:
	virtual ~Eternity();

	virtual Time& time() = 0;

	virtual Space& space() = 0;

	virtual std::unique_ptr<Spatiality> spatiality(Birth<Creation>) = 0;

	virtual std::unique_ptr<Temporality> temporality(Birth<Creation>) = 0;

	virtual matter::Soul enroll(SoulName name) = 0;

	virtual std::vector<matter::Soul> souls() const = 0;

	virtual matter::Fatherhood father(id::Soul father, id::Soul child, matter::Fatherhood::Line line) = 0;

	virtual std::vector<matter::Fatherhood> fatherhoods() const = 0;

	virtual matter::Word utter(id::Soul author, const Saying& saying) = 0;

	virtual matter::Word word(id::Word id) const = 0;

	virtual std::vector<matter::Word> words(const std::vector<id::Word>& ids) const = 0;

	virtual matter::Training utter(id::Word word, std::vector<Exercise> exercises) = 0;

	virtual std::vector<matter::Training> trainings(const std::vector<id::Word>& ids) const = 0;

	virtual matter::Remarked remark(id::Word remark, id::Word training, std::uint32_t exercise) = 0;

	virtual std::vector<matter::Remarked> remarked(const std::vector<id::Word>& trainings) const = 0;

protected:
	Eternity();

	Eternity(const Eternity&) = delete;
	Eternity& operator=(const Eternity&) = delete;

private:
	friend class Immanent<Eternity>;

	static Eternity& the();

	static Eternity* current_;
};


} // namespace will::domain
