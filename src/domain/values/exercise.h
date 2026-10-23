#pragma once

#include "values/approach.h"

#include <cstddef>
#include <string>
#include <vector>


namespace will::domain {


/// Named freely by the testator, who may give an instruction for it.
class Exercise {
public:
	static constexpr std::size_t MaxNameLength = 128;
	static constexpr std::size_t MaxApproaches = 100;
	static constexpr std::size_t MaxInstructionLength = 1000;

	Exercise(std::string name, std::vector<Approach> approaches, std::string instruction = {});

	const std::string& name() const noexcept { return name_; }
	const std::vector<Approach>& approaches() const noexcept { return approaches_; }
	const std::string& instruction() const noexcept { return instruction_; }

	bool operator==(const Exercise&) const = default;

private:
	std::string name_;
	std::vector<Approach> approaches_;
	std::string instruction_;
};


} // namespace will::domain
