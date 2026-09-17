#include "man.h"

#include "entities/dust.h"
#include "entities/spirit.h"

#include <utility>


namespace will::domain {


Man::Man(Embodiment embodiment)
	: Soul(embodiment.soul(), embodiment.name())
	, Vessel(embodiment.vessel(), embodiment.token())
	, id_(embodiment.man())
{
	Spirit::present();
	Dust::present();
}


} // namespace will::domain
