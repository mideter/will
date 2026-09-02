#include "ids/chat_id.h"
#include "ids/message_id.h"
#include "ids/user_id.h"

#include <cassert>
#include <cstdlib>
#include <stdexcept>


int main()
{
    using namespace will::domain;

    {
        const UserId id{42};
        assert(id.value() == 42);
    }

    try {
        UserId{0};
        return EXIT_FAILURE;
    } catch (const std::invalid_argument&) {
    }

    {
        const MessageId id{7};
        assert(id.value() == 7);
    }

    try {
        MessageId{0};
        return EXIT_FAILURE;
    } catch (const std::invalid_argument&) {
    }

    {
        assert(ChatId::global().value() == 0);
        assert(ChatId::global().is_global());
        assert(ChatId::global() == ChatId{0});
    }

    {
        const ChatId room{5};
        assert(room.value() == 5);
        assert(!room.is_global());
    }

    return EXIT_SUCCESS;
}
