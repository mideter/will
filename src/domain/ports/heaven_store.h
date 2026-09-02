#pragma once

#include "entities/god.h"
#include "values/god_name.h"

#include <string_view>
#include <vector>


namespace will::domain {


/// Long-term storage for gods. Runtime lookups use Heaven in memory.
class HeavenStore {
public:
    virtual ~HeavenStore() = default;

    virtual std::vector<God> load_all() = 0;
    virtual God insert(std::string_view device_token, GodName name) = 0;
};


} // namespace will::domain
