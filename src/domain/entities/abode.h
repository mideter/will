#pragma once

#include "ids/abode_id.h"


namespace will::domain {


/// Abode (Обитель) — место общения пользователей.
/// Single global abode is represented by AbodeId::global() (id 1).
struct Abode {
    AbodeId id = AbodeId::global();

    static Abode global() noexcept { return Abode{}; }
};


} // namespace will::domain
