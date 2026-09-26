#pragma once

#include "acts/utterance.h"
#include "identity/letter.h"
#include "identity/soul.h"
#include "ports/time.h"
#include "values/soul_name.h"
#include "values/saying.h"

#include <vector>


namespace will::domain {


class Space;


/// Eternity (Вечность) — who endures; indelible Saying and author.
/// Time and Space belong to Eternity; each is one, reached only from here.
/// Speaks in souls; embodiment in a vessel belongs to Temporality / World.
class Eternity {
public:
	virtual ~Eternity() = default;

	/// The one Time.
	virtual Time& time() = 0;

	/// The one Space.
	virtual Space& space() = 0;

	/// Enroll a soul in the book of life.
	virtual id::Soul enroll(SoulName name) = 0;

	/// Keep a Saying and author; returns the eternal utterance.
	virtual Utterance utter(id::Soul author, const Saying& saying) = 0;

	/// Utterance by id. Throws if unknown.
	virtual Utterance utterance(id::Letter id) const = 0;

	/// Utterances for the given ids (order preserved; skips unknown).
	virtual std::vector<Utterance> utterances(const std::vector<id::Letter>& ids) const = 0;
};


} // namespace will::domain
