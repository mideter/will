#include "entities/user_name.h"

#include <cassert>
#include <cstdlib>
#include <set>
#include <string>


int main()
{
    using namespace will::domain;

    assert(UserName::parse("abcdefgh"));
    assert(UserName::parse("abc12345"));
    assert(!UserName::parse(""));
    assert(!UserName::parse("short"));
    assert(!UserName::parse("ABCDEFGH"));
    assert(!UserName::parse("abcd-efg"));

    std::set<std::string> names;
    for (int i = 0; i < 64; ++i) {
        const UserName name = UserName::generate();
        assert(UserName::parse(name.text()));
        names.insert(std::string(name.text()));
    }
    assert(names.size() > 1);

    return EXIT_SUCCESS;
}
