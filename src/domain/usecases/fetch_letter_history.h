#pragma once

#include "ids/abode_id.h"
#include "ids/god_id.h"
#include "entities/letter.h"
#include "errors/domain_error.h"
#include "ports/heaven.h"
#include "ports/letter_repository.h"

#include <cstdint>
#include <string>
#include <variant>
#include <vector>


namespace will::domain {


struct FetchLetterHistoryInput {
    GodId god_id;
    AbodeId abode_id = AbodeId::global();
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
