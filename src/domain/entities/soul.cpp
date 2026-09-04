#include "soul.h"


namespace will::domain {


Soul::Soul(const id::Soul id, SoulName name)
    : id_(id)
    , name_(std::move(name))
{}


} // namespace will::domain
