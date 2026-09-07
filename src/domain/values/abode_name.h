#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>


namespace will::domain {


/// Public name of an abode: Latin letters a-z and digits, length in [MinLength, MaxLength].
class AbodeName {
public:
	static constexpr std::size_t MinLength = 1;
	static constexpr std::size_t MaxLength = 32;

	static std::optional<AbodeName> parse(std::string_view input);

	/// Name of the single global abode until a registry exists.
	static AbodeName global();

	std::string_view text() const noexcept { return value_; }

	bool operator==(const AbodeName&) const = default;

private:
	explicit AbodeName(std::string value);

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
