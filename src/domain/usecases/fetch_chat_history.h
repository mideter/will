#pragma once

#include "ids/chat_id.h"
#include "ids/user_id.h"
#include "entities/message.h"
#include "errors/domain_error.h"
#include "ports/message_repository.h"
#include "ports/user_repository.h"

#include <cstdint>
#include <string>
#include <variant>
#include <vector>


namespace will::domain {


struct FetchChatHistoryInput {
    UserId user_id;
    ChatId chat_id = ChatId::global();
    std::uint32_t limit = 0;
};


struct FetchChatHistoryItem {
    Message message;
    std::string author_name;
    bool is_mine = false;
};


struct FetchChatHistoryResult {
    std::vector<FetchChatHistoryItem> items;
};


class FetchChatHistory {
public:
    static constexpr std::uint32_t MaxHistoryRequestLimit = 1000;

    FetchChatHistory(MessageRepository& messages, UserRepository& users);

    std::variant<FetchChatHistoryResult, DomainError> execute(const FetchChatHistoryInput& input);

private:
    MessageRepository& messages_;
    UserRepository& users_;
};


} // namespace will::domain
