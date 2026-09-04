#pragma once

#include "entities/letter.h"
#include "identity/abode.h"
#include "identity/soul.h"

#include <string_view>
#include <vector>


namespace will::domain {


class LetterRepository {
public:
	virtual ~LetterRepository() = default;

	virtual Letter append(id::Abode abode, id::Soul author, std::string_view body, Timestamp ts) = 0;
	virtual std::vector<Letter> load_last(id::Abode abode, std::uint32_t limit) = 0;
};


} // namespace will::domain
