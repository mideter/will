#pragma once

#include "ids/abode_id.h"


namespace will::domain {


/// Abode (Обитель) — место общения пользователей.
/// Single global abode is represented by AbodeId::global().
struct Abode {
    AbodeId id = AbodeId::global();

    static constexpr Abode global() noexcept { return Abode{}; }
};


} // namespace will::domain
