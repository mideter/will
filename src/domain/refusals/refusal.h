#pragma once

#include <stdexcept>


namespace will::domain {


/// Refusal (Отказ) — the world does not let a man do what he would: it goes against
/// one of its rules, not against the code. Each refusal is the rule of the being that
/// keeps it — the gates, an abode, a tie, a training — and is declared beside it, in
/// its own words; whoever tells the man of it says why in his. Everything else thrown
/// as a logic_error is a breach of the world's own order — a fault of the code, never
/// a man's doing.
class Refusal : public std::logic_error {
protected:
	explicit Refusal(const char* words)
		: std::logic_error(words)
	{}
};


} // namespace will::domain
