#pragma once

#include <cstddef>
#include <string>


namespace will::domain {


class Saying {
public:
	static constexpr std::size_t MaxBodyLength = 4096;

	Saying(std::string body);
	Saying(const char* body);

	const std::string& body() const noexcept { return body_; }

	bool operator==(const Saying&) const = default;

private:
	std::string body_;
};


} // namespace will::domain
