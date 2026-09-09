#pragma once

#include "entities/letter.h"
#include "identity/abode.h"
#include "identity/soul.h"
#include "ports/eternity.h"

#include <string_view>
#include <vector>


namespace will::domain {


/// Temporality (Временность) — mutable Eternity; what happens in time.
/// World speaks with this one body: Heaven through the Eternity face,
/// living Witness through fix/letters.
class Temporality : public Eternity {
public:
	~Temporality() override = default;

	/// Fix a moment in time: take the present from Time and keep the letter.
	virtual void fix(id::Abode abode, id::Soul author, std::string_view body) = 0;

	/// Letters kept in this abode, bounded by limit.
	virtual std::vector<Letter> letters(id::Abode abode, std::uint32_t limit) = 0;
};


} // namespace will::domain
