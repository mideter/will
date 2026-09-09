#include "witness.h"

#include "entities/abode.h"

#include <map>
#include <stdexcept>


namespace will::domain {


Witness::Witness(Man&& man, Abode& abode) noexcept
	: Man(std::move(man))
	, abode_(&abode)
{}


void Witness::say(std::string_view body) const
{
	abode().inscribe(*this, body);
}


std::vector<RetoldLetter> Witness::hear(const std::uint32_t limit) const
{
	if (limit == 0)
		throw std::invalid_argument("Hear limit must be positive");

	const std::vector<Letter> rows = abode().letters(limit);
	const Heaven& heaven = abode().heaven();
	const id::Soul listener_soul = Soul::id();

	std::vector<RetoldLetter> items;
	items.reserve(rows.size());

	std::map<id::Soul, std::string> author_names;

	for (const Letter& row : rows) {
		std::string author_name;

		if (const auto cached = author_names.find(row.author_id()); cached != author_names.end()) {
			author_name = cached->second;
		} else if (heaven.knows(row.author_id())) {
			author_name = heaven.soul(row.author_id()).name().text();
			author_names.emplace(row.author_id(), author_name);
		} else {
			author_names.emplace(row.author_id(), std::string{});
		}

		items.push_back(RetoldLetter{row, std::move(author_name), row.author_id() == listener_soul});
	}

	return items;
}


} // namespace will::domain
