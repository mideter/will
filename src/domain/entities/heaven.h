#pragma once

#include "entities/god.h"
#include "ids/god_id.h"
#include "ports/heaven_store.h"
#include "values/device_token.h"
#include "values/god_name.h"

#include <mutex>
#include <optional>
#include <string_view>
#include <unordered_map>


namespace will::domain {


/// In-memory heavens — runtime registry of gods. Persistence goes through HeavenStore.
class Heaven {
public:
    explicit Heaven(HeavenStore& store);

    std::optional<God> find_by_id(GodId id) const;
    std::optional<God> find_by_device_token(std::string_view device_token) const;

    God create_god(std::string_view device_token, GodName name);

    void insert(God god);

private:
    HeavenStore& store_;
    mutable std::mutex mutex_;
    std::unordered_map<GodId, God> gods_by_id_;
    std::unordered_map<DeviceToken, GodId> id_by_token_;
};


} // namespace will::domain
