#include "abode_name.h"


namespace will::domain {


std::optional<AbodeName> AbodeName::parse(const std::string_view input)
{
	if (input.size() < MinLength || input.size() > MaxLength)
		return std::nullopt;

	for (const char c : input) {
		const bool ok = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
		if (!ok)
			return std::nullopt;
	}

	return AbodeName{std::string(input)};
}


AbodeName AbodeName::global()
{
	return AbodeName{"world"};
}


AbodeName::AbodeName(std::string value) : value_(std::move(value)) {}


} // namespace will::domain
