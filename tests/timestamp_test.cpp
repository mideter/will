#include "entities/timestamp.h"

#include <cassert>
#include <cstdlib>
#include <stdexcept>


int main()
{
    using namespace will::domain;

    {
        const Timestamp ts{1000};
        assert(ts.value() == 1000);
    }

    try {
        Timestamp{-1};
        return EXIT_FAILURE;
    } catch (const std::invalid_argument&) {
    }

    assert(Timestamp{0}.value() == 0);
    assert(Timestamp{}.value() >= 1'000'000'000'000'000'000LL);

    return EXIT_SUCCESS;
}
