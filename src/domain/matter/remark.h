#pragma once

#include "identity/word.h"
#include "matter/dating.h"
#include "matter/word.h"

#include <cstdint>


namespace will::domain::matter {


class Remarked {
public:
	Remarked(id::Word remark, id::Word training, std::uint32_t exercise);

	id::Word id() const noexcept { return remark_; }
	id::Word training() const noexcept { return training_; }
	std::uint32_t exercise() const noexcept { return exercise_; }

private:
	id::Word remark_;
	id::Word training_;
	std::uint32_t exercise_;
};


class Remark {
public:
	Remark(Word word, Remarked on, Dating dating);

	id::Word id() const noexcept { return word_.id(); }
	const Word& word() const noexcept { return word_; }
	id::Word training() const noexcept { return on_.training(); }
	std::uint32_t exercise() const noexcept { return on_.exercise(); }
	const Dating& dating() const noexcept { return dating_; }

private:
	Word word_;
	Remarked on_;
	Dating dating_;
};


} // namespace will::domain::matter
