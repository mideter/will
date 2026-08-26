#include "entities/timestamp.h"

#include <cassert>
#include <cstdlib>


int main()
{
    using namespace will::domain;

    {
        const auto parsed = Timestamp::parse(1000);
        assert(parsed);
        assert(parsed->value() == 1000);
    }

    assert(!Timestamp::parse(-1));
    assert(Timestamp::parse(0));
    assert(Timestamp{}.value() >= 1'000'000'000'000'000'000LL);

    return EXIT_SUCCESS;
}
