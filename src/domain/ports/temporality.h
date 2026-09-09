#pragma once

#include "entities/letter.h"
#include "identity/abode.h"
#include "identity/soul.h"
#include "ports/eternity.h"
#include "values/word.h"

#include <cstdint>
#include <vector>


namespace will::domain {


/// Temporality (Временность) — mutable Eternity; what happens in time.
/// Earth speaks with this face (as Heaven speaks with Eternity).
class Temporality : public Eternity {
public:
	static constexpr std::uint32_t MaxLetterLimit = 1000;

	~Temporality() override = default;

	/// Fix a word in time: take the present from Time and keep the letter.
	virtual void fix(id::Abode abode, id::Soul author, const Word& word) = 0;

	/// Letters kept in this abode, bounded by limit (history).
	virtual std::vector<Letter> letters(id::Abode abode, std::uint32_t limit) = 0;
};


} // namespace will::domain
