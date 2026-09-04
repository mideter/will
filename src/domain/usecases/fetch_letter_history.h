#pragma once

#include "identity/abode.h"
#include "identity/soul.h"
#include "entities/letter.h"
#include "errors/domain_error.h"
#include "entities/heaven.h"
#include "ports/letter_repository.h"

#include <cstdint>
#include <string>
#include <variant>
#include <vector>


namespace will::domain {


struct FetchLetterHistoryInput {
	id::Soul soul_id;
	id::Abode abode_id = id::Abode::global();
	std::uint32_t limit = 0;
};


struct FetchLetterHistoryItem {
	Letter letter;
	std::string author_name;
	bool is_mine = false;
};


struct FetchLetterHistoryResult {
	std::vector<FetchLetterHistoryItem> items;
};


class FetchLetterHistory {
public:
	static constexpr std::uint32_t MaxHistoryRequestLimit = 1000;

	FetchLetterHistory(LetterRepository& letters, Heaven& heaven);

	std::variant<FetchLetterHistoryResult, DomainError> execute(const FetchLetterHistoryInput& input);

private:
	LetterRepository& letters_;
	Heaven& heaven_;
};


} // namespace will::domain
