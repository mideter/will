#pragma once

#include "acts/utterance.h"
#include "identity/letter.h"
#include "identity/soul.h"
#include "ports/time.h"
#include "values/soul_name.h"
#include "values/word.h"

#include <vector>


namespace will::domain {


/// Eternity (Вечность) — who endures; indelible Word and author.
/// Time belongs to Eternity; without the eternal there is no present for the temporal.
/// Speaks in souls; embodiment in a vessel belongs to Temporality / World.
class Eternity {
public:
	virtual ~Eternity() = default;

	/// Time given by eternity.
	virtual Time& time() = 0;

	/// Enroll a soul in the book of life.
	virtual id::Soul enroll(SoulName name) = 0;

	/// Keep a word and author; returns the eternal utterance.
	virtual Utterance utter(id::Soul author, const Word& word) = 0;

	/// Utterance by id. Throws if unknown.
	virtual Utterance utterance(id::Letter id) const = 0;

	/// Utterances for the given ids (order preserved; skips unknown).
	virtual std::vector<Utterance> utterances(const std::vector<id::Letter>& ids) const = 0;
};


} // namespace will::domain
