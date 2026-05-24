#include "serverconfigerror.h"

#include <format>


namespace will {


ServerConfigError::ServerConfigError(const std::string_view field, const std::string_view reason)
    : std::invalid_argument(std::format("{}: {}", field, reason))
{}


} // namespace will
