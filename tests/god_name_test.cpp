#include "values/god_name.h"

#include <cassert>
#include <cstdlib>


int main()
{
    using namespace will::domain;

    assert(GodName::parse("abcdefgh"));
    assert(GodName::parse("abc12345"));
    assert(!GodName::parse(""));
    assert(!GodName::parse("short"));
    assert(!GodName::parse("ABCDEFGH"));
    assert(!GodName::parse("abcd-efg"));

    {
        const GodName name = GodName::generate();
        assert(GodName::parse(name.text()));
    }

    return EXIT_SUCCESS;
}
