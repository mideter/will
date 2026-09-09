#include "abode.h"

#include "entities/witness.h"

#include <algorithm>
#include <stdexcept>


namespace will::domain {


Abode::Abode(id::Abode id, AbodeName name, Temporality& temporality, Heaven& heaven)
	: id_(id)
	, name_(std::move(name))
	, temporality_(temporality)
	, heaven_(heaven)
{}


void Abode::admit(const Man& man)
{
	std::lock_guard lock(mutex_);
	dwellers_.insert(&man);
}


bool Abode::dwells(const Man& man) const
{
	std::lock_guard lock(mutex_);
	return dwellers_.contains(&man);
}


void Abode::inscribe(const Man& author, std::string_view body)
{
	const auto& witness = static_cast<const Witness&>(author);
	if (&witness.abode() != this)
		throw std::logic_error("Man is not observing this abode");

	temporality_.fix(id_, author.Soul::id(), body);
}


std::vector<Letter> Abode::letters(const std::uint32_t limit) const
{
	if (limit == 0)
		return {};

	const std::uint32_t capped_limit = std::min(limit, MaxLetterLimit);
	return temporality_.letters(id_, capped_limit);
}


} // namespace will::domain
