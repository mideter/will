#pragma once

#include <cstddef>
#include <string>


namespace will::domain {


/// Word (Слово) — that which the spirit utters.
class Word {
public:
	static constexpr std::size_t MaxBodyLength = 4096;

	/// Throws std::invalid_argument if body is empty or too long.
	explicit Word(std::string body);

	const std::string& body() const noexcept { return body_; }

	bool operator==(const Word&) const = default;

private:
	std::string body_;
};


} // namespace will::domain
