#include "fetch_chat_history.h"

#include <algorithm>


namespace will::domain {


FetchChatHistory::FetchChatHistory(MessageRepository& messages)
    : messages_(messages)
{}


std::variant<FetchChatHistoryResult, DomainError> FetchChatHistory::execute(const FetchChatHistoryInput& input)
{
    if (input.limit == 0) {
        return DomainError{DomainErrorCode::InvalidArgument};
    }

    const std::uint32_t capped_limit = std::min(input.limit, MaxHistoryRequestLimit);
    const std::vector<Message> rows = messages_.load_last(input.chat_id, capped_limit);

    FetchChatHistoryResult result;
    result.items.reserve(rows.size());

    for (const Message& row : rows) {
        const bool is_mine = row.author_id == input.account.user_id;
        result.items.push_back(FetchChatHistoryItem{row, is_mine});
    }

    return result;
}


} // namespace will::domain
