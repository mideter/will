#pragma once

#include "entities/auth_token.h"

#include <string>


namespace will {


/** Loads or creates a persistent device token on the local filesystem. */
class DeviceTokenStore {
public:
    static domain::AuthToken load_or_create(const std::string& path);
};


} // namespace will
