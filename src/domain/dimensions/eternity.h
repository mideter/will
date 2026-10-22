#pragma once

#include "matter/fatherhood.h"
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


/// What endures: souls, their words, fatherhood. Time and Space
/// are reached only from it; it realises Spatiality and Temporality for a Creation.
class Eternity {
public:
	virtual ~Eternity();

	virtual Time& time() = 0;

	virtual Space& space() = 0;

	virtual std::unique_ptr<Spatiality> spatiality(Birth<Creation>) = 0;

	virtual std::unique_ptr<Temporality> temporality(Birth<Creation>) = 0;

	virtual matter::Soul enroll(SoulName name) = 0;

	virtual std::vector<matter::Soul> souls() const = 0;

	/// By spirit it replaces the child's earlier father; by flesh it refuses
	/// (std::logic_error) if the child already has one.
	virtual matter::Fatherhood father(id::Soul father, id::Soul child, matter::Fatherhood::Line line) = 0;

	virtual std::vector<matter::Fatherhood> fatherhoods() const = 0;

	virtual matter::Word utter(id::Soul author, const Saying& saying) = 0;

	/// Throws if unknown.
	virtual matter::Word word(id::Word id) const = 0;

	/// Order preserved; skips unknown.
	virtual std::vector<matter::Word> words(const std::vector<id::Word>& ids) const = 0;

	/// Refuses (std::logic_error) if the word already has its exercises.
	virtual matter::Training utter(id::Word word, std::vector<Exercise> exercises) = 0;

	virtual std::vector<matter::Training> trainings(const std::vector<id::Word>& ids) const = 0;

protected:
	Eternity();

	Eternity(const Eternity&) = delete;
	Eternity& operator=(const Eternity&) = delete;

private:
	friend class Immanent<Eternity>;

	/// Throws if no Eternity is realised.
	static Eternity& the();

	static Eternity* current_;
};


} // namespace will::domain
