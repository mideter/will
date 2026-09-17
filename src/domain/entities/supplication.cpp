#include "supplication.h"

#include "entities/soul.h"
#include "entities/spirit.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Supplication::Supplication(const id::Supplication id, const Soul& suppliant, const Soul& addressee,
						   const SupplicationStatus status, Timestamp created_at)
	: id_(id)
	, suppliant_id_(suppliant.id())
	, addressee_id_(addressee.id())
	, status_(status)
	, created_at_(std::move(created_at))
{
	if (suppliant_id_ == addressee_id_)
		throw std::invalid_argument("supplication requires distinct suppliant and addressee");
	if (!Spirit::knows(suppliant_id_) || !Spirit::knows(addressee_id_))
		throw std::invalid_argument("supplication requires souls known to Heaven");
}


const Soul& Supplication::suppliant() const
{
	return Spirit::known(suppliant_id_);
}


const Soul& Supplication::addressee() const
{
	return Spirit::known(addressee_id_);
}


} // namespace will::domain
