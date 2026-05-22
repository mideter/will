#include "serverconfigerror.h"

#include <string>


namespace will {


ServerConfigError::ServerConfigError(const char* field, const char* reason)
    : std::invalid_argument(std::string(field) + ": " + reason)
{}


} // namespace will
