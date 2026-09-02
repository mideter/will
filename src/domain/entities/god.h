#pragma once

#include "ids/god_id.h"
#include "values/god_name.h"


namespace will::domain {


/// God (Бог) — participant of an abode.
class God {
public:
    God(GodId id, GodName name);

    GodId id() const noexcept { return id_; }
    const GodName& name() const noexcept { return name_; }

    bool operator==(const God&) const = default;

private:
    GodId id_;
    GodName name_;
};


} // namespace will::domain
