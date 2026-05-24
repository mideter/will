#pragma once

#include <span>


namespace will {


class CliOption;

std::span<const CliOption> AllServerCliOptions();


} // namespace will
