module;

#include <cstdint>
#include <variant>
#include <vector>

export module will.domain.fetch_chat_history;

export import will.domain.account;
export import will.domain.chat_id;
export import will.domain.domain_error;
export import will.domain.message;
export import will.domain.message_repository;

export namespace will::domain {


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
