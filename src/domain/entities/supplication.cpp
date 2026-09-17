#include "supplication.h"

#include "entities/soul.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Supplication::Supplication(const id::Supplication id, const Soul& suppliant, const Soul& addressee,
						   const SupplicationStatus status, Timestamp created_at)
	: id_(id)
	, suppliant_(suppliant)
	, addressee_(addressee)
	, status_(status)
	, created_at_(std::move(created_at))
{
	if (suppliant.id() == addressee.id())
		throw std::invalid_argument("supplication requires distinct suppliant and addressee");

	if (!suppliant.heaven().knows(suppliant.id()) || !suppliant.heaven().knows(addressee.id()))
		throw std::invalid_argument("supplication requires souls known to Heaven");
}


} // namespace will::domain
