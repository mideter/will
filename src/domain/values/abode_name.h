#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>


namespace will::domain {


/// Public name of an abode: Latin letters a-z and digits, length in [MinLength, MaxLength].
class AbodeName {
public:
	static constexpr std::size_t MinLength = 1;
	static constexpr std::size_t MaxLength = 32;

	/// Throws std::invalid_argument if input is not a valid abode name.
	explicit AbodeName(std::string_view input);

	std::string_view text() const noexcept { return value_; }

	bool operator==(const AbodeName&) const = default;
	bool operator==(std::string_view other) const noexcept { return value_ == other; }

private:
	std::string value_;
};


} // namespace will::domain


template <>
struct std::hash<will::domain::AbodeName> {
	std::size_t operator()(const will::domain::AbodeName& name) const noexcept
	{
		return std::hash<std::string_view>{}(name.text());
	}
};
