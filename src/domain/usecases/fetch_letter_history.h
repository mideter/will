#pragma once

#include "ids/abode_id.h"
#include "ids/user_id.h"
#include "entities/letter.h"
#include "errors/domain_error.h"
#include "ports/letter_repository.h"
#include "ports/user_repository.h"

#include <cstdint>
#include <string>
#include <variant>
#include <vector>


namespace will::domain {


struct FetchLetterHistoryInput {
    UserId user_id;
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

    FetchLetterHistory(LetterRepository& letters, UserRepository& users);

    std::variant<FetchLetterHistoryResult, DomainError> execute(const FetchLetterHistoryInput& input);

private:
    LetterRepository& letters_;
    UserRepository& users_;
};


} // namespace will::domain
