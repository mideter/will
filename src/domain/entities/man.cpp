#include "man.h"

#include "entities/dust.h"
#include "entities/spirit.h"

#include <utility>


namespace will::domain {


Man::Man(const id::Man id, const id::Soul soul_id, SoulName name, const id::Vessel vessel_id,
		 DeviceToken token)
	: Soul(soul_id, std::move(name))
	, Vessel(vessel_id, std::move(token))
	, id_(id)
{
	Spirit::present();
	Dust::present();
}


} // namespace will::domain
