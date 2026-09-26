#pragma once

#include <cstddef>
#include <string>


namespace will::domain {


/// Saying (Речение) — external manifestation of Word; text the spirit utters.
class Saying {
public:
	static constexpr std::size_t MaxBodyLength = 4096;

	/// Throws std::invalid_argument if body is empty or too long.
	Saying(std::string body);

	const std::string& body() const noexcept { return body_; }

	bool operator==(const Saying&) const = default;

private:
	std::string body_;
};


} // namespace will::domain
