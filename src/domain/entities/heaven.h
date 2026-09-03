#pragma once

#include "entities/god.h"
#include "ids/god.h"
#include "ports/eternity.h"
#include "values/god_name.h"

#include <mutex>
#include <optional>
#include <unordered_map>


namespace will::domain {


/// In-memory heavens — runtime registry of gods. Persistence goes through Eternity.
class Heaven {
public:
    explicit Heaven(Eternity& eternity);

    std::optional<God> find_by_id(id::God id) const;

    void insert(God god);

private:
    mutable std::mutex mutex_;
    std::unordered_map<id::God, God> gods_by_id_;
};


} // namespace will::domain
