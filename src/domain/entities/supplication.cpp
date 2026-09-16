#include "supplication.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Supplication::Supplication(const id::Supplication id, const id::Soul suppliant, const id::Soul addressee,
						   const SupplicationStatus status, Timestamp created_at)
	: id_(id)
	, suppliant_id_(suppliant)
	, addressee_id_(addressee)
	, status_(status)
	, created_at_(std::move(created_at))
{
	if (suppliant_id_ == addressee_id_)
		throw std::invalid_argument("supplication requires distinct suppliant and addressee");
}


} // namespace will::domain
