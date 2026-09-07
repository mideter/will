#include "abode.h"

#include <algorithm>
#include <map>


namespace will::domain {


Abode::Abode(id::Abode id, AbodeName name, Temporality& temporality, Heaven& heaven)
	: id_(id)
	, name_(std::move(name))
	, temporality_(temporality)
	, heaven_(heaven)
{}


void Abode::admit(const Soul& soul)
{
	std::lock_guard lock(mutex_);
	members_.insert(&soul);
}


bool Abode::shelters(const Soul& soul) const
{
	std::lock_guard lock(mutex_);
	return members_.contains(&soul);
}


Letter Abode::inscribe(id::Soul author, std::string_view body)
{
	return temporality_.fix(id_, author, body);
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
