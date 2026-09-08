#include "abode.h"

#include <algorithm>
#include <map>
#include <stdexcept>
#include <utility>


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


Shelter& Abode::shelter(Man& man)
{
	if (Shelter* current = man.shelter(); current && &current->abode() == this)
		return *current;

	if (Shelter* current = man.shelter())
		current->abode().release_shelter(man);

	std::lock_guard lock(mutex_);
	if (!dwellers_.contains(&man))
		throw std::logic_error("Man does not dwell in this abode");

	if (const auto it = shelters_.find(&man); it != shelters_.end()) {
		man.bind_shelter(it->second.get());
		return *it->second;
	}

	auto cover = std::make_unique<Shelter>(*this, man);
	Shelter& ref = *cover;
	man.bind_shelter(&ref);
	shelters_.emplace(&man, std::move(cover));
	return ref;
}


void Abode::release_shelter(Man& man)
{
	std::lock_guard lock(mutex_);
	const auto it = shelters_.find(&man);
	if (it == shelters_.end())
		return;

	if (man.shelter() == it->second.get())
		man.bind_shelter(nullptr);

	shelters_.erase(it);
}


bool Abode::shelters(const Man& man) const
{
	const Shelter* cover = man.shelter();
	return cover != nullptr && &cover->abode() == this;
}


Letter Abode::inscribe(const Man& author, std::string_view body)
{
	if (!shelters(author))
		throw std::logic_error("Man has no shelter in this abode");

	return temporality_.fix(id_, author.soul_id(), body);
}


std::variant<std::vector<RetoldLetter>, DomainError> Abode::retell(id::Soul soul, std::uint32_t limit) const
{
	if (limit == 0)
		return DomainError{DomainErrorCode::InvalidArgument};

	const std::uint32_t capped_limit = std::min(limit, MaxRetellLimit);
	const std::vector<Letter> rows = temporality_.letters(id_, capped_limit);

	std::vector<RetoldLetter> items;
	items.reserve(rows.size());

	std::map<id::Soul, std::string> author_names;

	for (const Letter& row : rows) {
		std::string author_name;

		if (const auto cached = author_names.find(row.author_id()); cached != author_names.end()) {
			author_name = cached->second;
		} else if (heaven_.knows(row.author_id())) {
			author_name = heaven_.soul(row.author_id()).name().text();
			author_names.emplace(row.author_id(), author_name);
		} else {
			author_names.emplace(row.author_id(), std::string{});
		}

		items.push_back(RetoldLetter{row, std::move(author_name), row.author_id() == soul});
	}

	return items;
}


} // namespace will::domain
