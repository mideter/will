#include "god.h"


namespace will::domain {


God::God(const id::God id, GodName name)
    : id_(id)
    , name_(std::move(name))
{}


} // namespace will::domain
