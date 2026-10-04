#include "place.h"

#include "horizons/life.h"
#include "horizons/space.h"

#include <stdexcept>
#include <utility>


namespace will::domain {


Place::Place()
	: id_(id::Place{1})
{
	throw std::logic_error("Place default ctor is only for virtual-base faces");
}


Place::Place(const id::Place id) noexcept
	: id_(id)
{}


const Place& Place::of(const id::Place id)
{
	return Space::the().place(id);
}


std::vector<matter::Letter> Place::kept_letters() const
{
	return Life::the().letters(id());
}


std::vector<matter::Behest> Place::kept_behests() const
{
	return Life::the().behests(id());
}


std::vector<matter::Deed> Place::kept_deeds() const
{
	return Life::the().deeds(id());
}


std::vector<std::shared_ptr<const Word>> Place::living_words(bool& whole) const
{
	Life::Remembered remembered = Life::the().remembered(id());
	whole = remembered.whole;
	return std::move(remembered.words);
}


void Place::remember(const std::vector<std::shared_ptr<const Word>>& words) const
{
	Life::the().remember(id(), words);
}


void Place::remember(const std::shared_ptr<const Word>& word) const
{
	Life::the().remember(id(), word);
}


} // namespace will::domain
