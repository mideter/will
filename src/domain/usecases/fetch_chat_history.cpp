#include "fetch_chat_history.h"

#include <algorithm>
#include <map>


namespace will::domain {


FetchChatHistory::FetchChatHistory(MessageRepository& messages, UserRepository& users)
    : messages_(messages)
    , users_(users)
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

    std::map<UserId, std::string> author_names;
    for (const Message& row : rows) {
        std::string author_name;
        if (const auto cached = author_names.find(row.author_id()); cached != author_names.end()) {
            author_name = cached->second;
        } else if (const std::optional<User> author = users_.find_by_id(row.author_id())) {
            author_name = author->name().text();
            author_names.emplace(row.author_id(), author_name);
        } else {
            author_names.emplace(row.author_id(), std::string{});
        }

        const bool is_mine = row.author_id() == input.user_id;
        result.items.push_back(FetchChatHistoryItem{row, std::move(author_name), is_mine});
    }

    return result;
}


} // namespace will::domain
