#pragma once

#include "identity/abode.h"


namespace will::domain {


/// Abode (Обитель) — place of communion in the World.
/// Single global abode is represented by id::Abode::global() (id 1).
struct Abode {
    id::Abode id = id::Abode::global();

    static Abode global() noexcept { return Abode{}; }
};


} // namespace will::domain
