#pragma once

#include "entities/message.h"
#include "ids/abode_id.h"
#include "ids/user_id.h"

#include <string_view>
#include <vector>


namespace will::domain {


class MessageRepository {
public:
    virtual ~MessageRepository() = default;

    virtual Message append(AbodeId abode, UserId author, std::string_view body, Timestamp ts) = 0;
    virtual std::vector<Message> load_last(AbodeId abode, std::uint32_t limit) = 0;
};


} // namespace will::domain
