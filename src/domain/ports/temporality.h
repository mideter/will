#pragma once

#include "entities/letter.h"
#include "identity/abode.h"
#include "identity/soul.h"

#include <string_view>
#include <vector>


namespace will::domain {


/// Temporality (Временность) — what happens in time. Pair to Eternity; World unites both.
class Temporality {
public:
	virtual ~Temporality() = default;

	/// Fix a moment in time: capture now and keep the letter.
	virtual Letter fix(id::Abode abode, id::Soul author, std::string_view body) = 0;

	virtual std::vector<Letter> load_last(id::Abode abode, std::uint32_t limit) = 0;
};


} // namespace will::domain
