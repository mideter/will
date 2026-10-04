#include "tie.h"

#include "words/deed.h"
#include "immanents/soul.h"
#include "horizons/space.h"
#include "immanents/testator.h"
#include "matter/deed.h"
#include "properties/immanent.h"

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>


namespace will::domain {


Tie::Tie(Birth<Novice>, matter::Tie kept)
	: Place(id::Place{kept.id().value()})
	, Obedience(Soul::of(kept.testator()))
	, Shepherding(Soul::of(kept.novice()))
{
	Immanent<Space>::present<Place>();
}


Tie::~Tie()
{
	testator_.release(*this);
}


const Testator& Tie::testator() const
{
	return testator_;
}


const Novice& Tie::novice() const
{
	return novice_;
}


std::vector<std::shared_ptr<const Deed>> Tie::deeds(const Novice& asker) const
{
	if (asker.Soul::id() != novice_.Soul::id() && asker.Soul::id() != testator_.Soul::id())
		throw std::logic_error("not a side of this tie");

	std::vector<std::shared_ptr<const Deed>> shown;
	for (matter::Deed& kept : kept_deeds())
		shown.push_back(std::make_shared<const Deed>(Birth<Tie>{}, std::move(kept)));

	return shown;
}


} // namespace will::domain
