#include "entities/user_name.h"

#include <cassert>
#include <cstdlib>
#include <set>
#include <string>


int main()
{
    using namespace will::domain;

    assert(UserName::is_valid("abcdefgh"));
    assert(UserName::is_valid("abc12345"));
    assert(!UserName::is_valid(""));
    assert(!UserName::is_valid("short"));
    assert(!UserName::is_valid("ABCDEFGH"));
    assert(!UserName::is_valid("abcd-efg"));

    std::set<std::string> names;
    for (int i = 0; i < 64; ++i) {
        const std::string name = UserName::generate();
        assert(UserName::is_valid(name));
        names.insert(name);
    }
    assert(names.size() > 1);

    return EXIT_SUCCESS;
}
