#pragma once

#include "ids/god.h"
#include "values/god_name.h"


namespace will::domain {


/// God (Бог) — participant of an abode.
class God {
public:
    God(id::God id, GodName name);

    id::God id() const noexcept { return id_; }
    const GodName& name() const noexcept { return name_; }

    bool operator==(const God&) const = default;

private:
    id::God id_;
    GodName name_;
};


} // namespace will::domain
