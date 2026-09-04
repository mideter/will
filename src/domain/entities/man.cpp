#include "man.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Man::Man(Soul&& soul, Vessel&& vessel)
    : soul_(std::move(soul))
    , vessel_(std::move(vessel))
{
    if (vessel_.soul_id() != soul_.id())
        throw std::invalid_argument("Man: vessel does not belong to soul");
}


} // namespace will::domain
