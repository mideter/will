#pragma once

#include "entities/god.h"
#include "ids/god_id.h"
#include "values/god_name.h"

#include <optional>
#include <string_view>


namespace will::domain {


class GodRepository {
public:
    virtual ~GodRepository() = default;

    virtual std::optional<God> find_by_device_token(std::string_view device_token) = 0;
    virtual std::optional<God> find_by_id(GodId id) = 0;
    virtual God create_god(std::string_view device_token, GodName name) = 0;
};


} // namespace will::domain
