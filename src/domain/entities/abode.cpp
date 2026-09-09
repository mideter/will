#include "abode.h"

#include <algorithm>
#include <map>
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


std::variant<std::vector<RetoldLetter>, DomainError> Abode::retell(const Witness& listener,
																   const std::uint32_t limit) const
{
	if (&listener.abode() != this)
		return DomainError{DomainErrorCode::Unauthorized};

	if (limit == 0)
		return DomainError{DomainErrorCode::InvalidArgument};

	const std::uint32_t capped_limit = std::min(limit, MaxRetellLimit);
	const std::vector<Letter> rows = temporality_.letters(id_, capped_limit);

	std::vector<RetoldLetter> items;
	items.reserve(rows.size());

	std::map<id::Soul, std::string> author_names;
	const id::Soul listener_soul = listener.Soul::id();

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

		items.push_back(RetoldLetter{row, std::move(author_name), row.author_id() == listener_soul});
	}

	return items;
}


} // namespace will::domain
