#include "values/soul_name.h"

#include <cassert>
#include <cstdlib>


int main()
{
    using namespace will::domain;

    assert(SoulName::parse("abcdefgh"));
    assert(SoulName::parse("abc12345"));
    assert(!SoulName::parse(""));
    assert(!SoulName::parse("short"));
    assert(!SoulName::parse("ABCDEFGH"));
    assert(!SoulName::parse("abcd-efg"));

    {
        const SoulName name = SoulName::generate();
        assert(SoulName::parse(name.text()));
    }

    return EXIT_SUCCESS;
}
