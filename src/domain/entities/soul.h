#pragma once

#include "ids/soul.h"
#include "values/soul_name.h"


namespace will::domain {


/// Soul (Душа) — participant of an abode.
class Soul {
public:
    Soul(id::Soul id, SoulName name);

    id::Soul id() const noexcept { return id_; }
    const SoulName& name() const noexcept { return name_; }

    bool operator==(const Soul&) const = default;

private:
    id::Soul id_;
    SoulName name_;
};


} // namespace will::domain
