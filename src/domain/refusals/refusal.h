#pragma once

#include <stdexcept>


namespace will::domain {


class Refusal : public std::logic_error {
protected:
	explicit Refusal(const char* words)
		: std::logic_error(words)
	{}
};


} // namespace will::domain
