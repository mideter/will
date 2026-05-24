#pragma once

#include <span>


namespace will {


class CliOption;

extern const CliOption HelpCliOption;

std::span<const CliOption> ServerCliOptions();


} // namespace will
