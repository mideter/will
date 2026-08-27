#pragma once

#include "entities/device_token.h"

#include <string>


namespace will {


/** Loads or creates a persistent device token on the local filesystem. */
class DeviceTokenStore {
public:
    static domain::DeviceToken load_or_create(const std::string& path);
};


} // namespace will
