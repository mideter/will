#include "ids/abode_id.h"
#include "ids/letter_id.h"
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
        const LetterId id{7};
        assert(id.value() == 7);
    }

    try {
        LetterId{0};
        return EXIT_FAILURE;
    } catch (const std::invalid_argument&) {
    }

    {
        assert(AbodeId::global() == AbodeId{1});
    }

    try {
        AbodeId{0};
        return EXIT_FAILURE;
    } catch (const std::invalid_argument&) {
    }

    {
        const AbodeId abode{5};
        assert(abode.value() == 5);
        assert(abode != AbodeId::global());
    }

    return EXIT_SUCCESS;
}
