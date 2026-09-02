#include "entities/message.h"
#include "ids/message_id.h"

#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <string>


int main()
{
    using namespace will::domain;

    {
        const MessageId id{1};
        assert(id.value() == 1);
    }

    try {
        MessageId{0};
        return EXIT_FAILURE;
    } catch (const std::invalid_argument&) {
    }

    {
        const Message msg{MessageId{1}, AbodeId::global(), UserId{7}, "hello", Timestamp{100}};
        assert(msg.id() == MessageId{1});
        assert(msg.abode_id() == AbodeId::global());
        assert(msg.author_id() == UserId{7});
        assert(msg.body() == "hello");
        assert(msg.created_at() == Timestamp{100});
    }

    try {
        Message{MessageId{1}, AbodeId::global(), UserId{0}, "x", Timestamp{0}};
        return EXIT_FAILURE;
    } catch (const std::invalid_argument&) {
    }

    try {
        Message{MessageId{1}, AbodeId::global(), UserId{1}, "", Timestamp{0}};
        return EXIT_FAILURE;
    } catch (const std::invalid_argument&) {
    }

    try {
        Message{MessageId{1}, AbodeId::global(), UserId{1}, std::string(Message::MaxBodyLength + 1, 'a'),
                Timestamp{0}};
        return EXIT_FAILURE;
    } catch (const std::invalid_argument&) {
    }

    {
        const Message max_body{MessageId{1}, AbodeId::global(), UserId{1},
                               std::string(Message::MaxBodyLength, 'a'), Timestamp{0}};
        assert(max_body.body().size() == Message::MaxBodyLength);
    }

    return EXIT_SUCCESS;
}
