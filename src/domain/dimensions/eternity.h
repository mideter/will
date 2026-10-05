#pragma once

#include "matter/soul.h"
#include "matter/word.h"
#include "identity/word.h"
#include "identity/soul.h"
#include "horizons/time.h"
#include "properties/birth.h"
#include "values/soul_name.h"
#include "values/saying.h"

#include <memory>
#include <vector>


namespace will::domain {


class Creation;
class Life;
class Space;
class Spatiality;
class Temporality;


/// Eternity (Вечность) — who endures; indelible Saying and author.
/// Time and Space belong to Eternity; each is one, reached only from here.
/// Spatiality and Temporality arise in Creation: Eternity realises them at its
/// asking; their matter outlives every Creation.
/// Speaks in souls; dwelling in a vessel belongs to Temporality / World.
/// One Eternity; Life proceeds from it.
class Eternity {
public:
	virtual ~Eternity();

	virtual Time& time() = 0;

	virtual Space& space() = 0;

	/// Realise Spatiality for the Creation that asks.
	virtual std::unique_ptr<Spatiality> spatiality(Birth<Creation>) = 0;

	/// Realise Temporality for the Creation that asks.
	virtual std::unique_ptr<Temporality> temporality(Birth<Creation>) = 0;

	/// Enroll a soul in the book of life.
	virtual matter::Soul enroll(SoulName name) = 0;

	/// All souls kept (for Creation awaken).
	virtual std::vector<matter::Soul> souls() const = 0;

	/// Keep a Saying and author; returns the eternal word.
	virtual matter::Word utter(id::Soul author, const Saying& saying) = 0;

	/// Throws if unknown.
	virtual matter::Word word(id::Word id) const = 0;

	/// Order preserved; skips unknown.
	virtual std::vector<matter::Word> words(const std::vector<id::Word>& ids) const = 0;

protected:
	/// The one realised Eternity declares itself.
	Eternity();

	Eternity(const Eternity&) = delete;
	Eternity& operator=(const Eternity&) = delete;

private:
	friend class Life;

	/// Throws if no Eternity is realised.
	static Eternity& the();

	static Eternity* current_;
};


} // namespace will::domain
