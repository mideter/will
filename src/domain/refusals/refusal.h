#pragma once

#include <stdexcept>


namespace will::domain {


/// The world does not let a man do what he would: a rule of the being
/// that keeps it, declared by being in refusals/. Any other logic_error is a fault of
/// the code, never a man's doing.
class Refusal : public std::logic_error {
protected:
	explicit Refusal(const char* words)
		: std::logic_error(words)
	{}
};


} // namespace will::domain
