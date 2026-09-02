#include "ids/abode_id.h"
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
        assert(AbodeId::global().value() == 0);
        assert(AbodeId::global().is_global());
        assert(AbodeId::global() == AbodeId{0});
    }

    {
        const AbodeId abode{5};
        assert(abode.value() == 5);
        assert(!abode.is_global());
    }

    return EXIT_SUCCESS;
}
