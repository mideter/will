#pragma once

#include "entities/chat_id.h"
#include "entities/message.h"
#include "entities/user_id.h"

#include <string_view>
#include <vector>


namespace will::domain {


class MessageRepository {
public:
    virtual ~MessageRepository() = default;

    virtual Message append(ChatId chat, UserId author, std::string_view body, Timestamp ts) = 0;
    virtual std::vector<Message> load_last(ChatId chat, std::uint32_t limit) = 0;
};


} // namespace will::domain
