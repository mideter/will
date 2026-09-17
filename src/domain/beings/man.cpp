#include "man.h"

#include "beings/dust.h"
#include "beings/spirit.h"

#include <utility>


namespace will::domain {


Man::Man(Embodiment embodiment)
	: Soul(embodiment.soul(), embodiment.name())
	, Vessel(embodiment.vessel(), embodiment.token())
	, Hypostasis(embodiment.man())
{
	Spirit::present();
	Dust::present();
}


} // namespace will::domain
