module;

#include <cstdint>
#include <string_view>
#include <vector>

export module will.domain.message_repository;

export import will.domain.chat_id;
export import will.domain.message;
export import will.domain.user_id;

export namespace will::domain {


class MessageRepository {
public:
    virtual ~MessageRepository() = default;

    virtual Message append(ChatId chat, UserId author, std::string_view body, Timestamp ts) = 0;
    virtual std::vector<Message> load_last(ChatId chat, std::uint32_t limit) = 0;
};


} // namespace will::domain
