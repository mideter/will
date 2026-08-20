module;

#include <cstdint>
#include <string>

export module will.domain.message;

export import will.domain.chat_id;
export import will.domain.timestamp;
export import will.domain.user_id;

export namespace will::domain {


struct Message {
    std::uint64_t id = 0;
    ChatId chat_id = ChatId::global();
    UserId author_id{};
    std::string body;
    Timestamp created_at{};
    std::string author_name; // filled at send/load time; not a messages column
};


} // namespace will::domain
