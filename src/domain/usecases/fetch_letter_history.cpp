#include "fetch_letter_history.h"

#include <algorithm>
#include <map>


namespace will::domain {


FetchLetterHistory::FetchLetterHistory(LetterRepository& letters, Heaven& heaven)
	: letters_(letters)
	, heaven_(heaven)
{}


std::variant<FetchLetterHistoryResult, DomainError> FetchLetterHistory::execute(const FetchLetterHistoryInput& input)
{
	if (input.limit == 0) {
		return DomainError{DomainErrorCode::InvalidArgument};
	}

	const std::uint32_t capped_limit = std::min(input.limit, MaxHistoryRequestLimit);
	const std::vector<Letter> rows = letters_.load_last(input.abode_id, capped_limit);

	FetchLetterHistoryResult result;
	result.items.reserve(rows.size());

	std::map<id::Soul, std::string> author_names;
	for (const Letter& row : rows) {
		std::string author_name;
		if (const auto cached = author_names.find(row.author_id()); cached != author_names.end()) {
			author_name = cached->second;
		} else if (const std::optional<Soul> author = heaven_.find_by_id(row.author_id())) {
			author_name = author->name().text();
			author_names.emplace(row.author_id(), author_name);
		} else {
			author_names.emplace(row.author_id(), std::string{});
		}

		const bool is_mine = row.author_id() == input.soul_id;
		result.items.push_back(FetchLetterHistoryItem{row, std::move(author_name), is_mine});
	}

	return result;
}


} // namespace will::domain
