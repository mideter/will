#include "abode.h"

#include "beings/immanents/man.h"
#include "beings/immanents/witness.h"
#include "beings/space.h"
#include "properties/immanent.h"

#include <utility>


namespace will::domain {


Abode::Abode(const id::Abode id, AbodeName name)
	: Place(id::Place{id.value()})
	, name_(std::move(name))
	, mutex_(std::make_unique<std::mutex>())
{
	Immanent<Space>::present<Abode>();
}


Abode::Abode(Abiding abiding)
	: Abode(abiding.id(), abiding.name())
{}


void Abode::admit(const Man& man)
{
	std::lock_guard lock(*mutex_);
	dwellers_.insert(&man);
}


bool Abode::dwells(const Man& man) const
{
	std::lock_guard lock(*mutex_);
	return dwellers_.contains(&man);
}


void Abode::observe(const Witness& witness)
{
	std::lock_guard lock(*mutex_);
	witnesses_.insert(&witness);
}


bool Abode::observed_by(const Witness& witness) const
{
	std::lock_guard lock(*mutex_);
	return witnesses_.contains(&witness);
}


std::vector<std::reference_wrapper<const Witness>> Abode::witnesses() const
{
	std::lock_guard lock(*mutex_);
	std::vector<std::reference_wrapper<const Witness>> out;
	out.reserve(witnesses_.size());
	for (const Witness* w : witnesses_) {
		if (w)
			out.emplace_back(*w);
	}
	return out;
}


} // namespace will::domain
