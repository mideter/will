#include "supplication.h"

#include "beings/soul.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Supplication::Supplication(const id::Supplication id, const Soul& suppliant, const Soul& testator,
						   const SupplicationStatus status, Timestamp created_at)
	: id_(id)
	, suppliant_(suppliant)
	, testator_(testator)
	, status_(status)
	, created_at_(std::move(created_at))
{
	if (suppliant.id() == testator.id())
		throw std::invalid_argument("supplication requires distinct suppliant and testator");
}


} // namespace will::domain
