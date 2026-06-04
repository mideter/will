#pragma once

#include "entities/account.h"
#include "entities/chat_id.h"
#include "entities/message.h"
#include "errors/domain_error.h"
#include "ports/message_repository.h"

#include <cstdint>
#include <variant>
#include <vector>


namespace will::domain {


struct FetchChatHistoryInput {
    Account account;
    ChatId chat_id = ChatId::global();
    std::uint32_t limit = 0;
};


struct FetchChatHistoryItem {
    Message message;
    bool is_mine = false;
};


struct FetchChatHistoryResult {
    std::vector<FetchChatHistoryItem> items;
};


class FetchChatHistory {
public:
    static constexpr std::uint32_t MaxHistoryRequestLimit = 1000;

    explicit FetchChatHistory(MessageRepository& messages);

    std::variant<FetchChatHistoryResult, DomainError> execute(const FetchChatHistoryInput& input);

private:
    MessageRepository& messages_;
};


} // namespace will::domain
