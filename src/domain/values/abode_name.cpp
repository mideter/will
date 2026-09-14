#include "abode_name.h"

#include <stdexcept>


namespace will::domain {


AbodeName::AbodeName(const std::string_view input)
{
	if (input.size() < MinLength || input.size() > MaxLength)
		throw std::invalid_argument("Invalid abode name");

	for (const char c : input) {
		const bool ok = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
		if (!ok)
			throw std::invalid_argument("Invalid abode name");
	}

	value_ = std::string(input);
}


} // namespace will::domain
