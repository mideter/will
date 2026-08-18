#include <cassert>
#include <cstdlib>

import will.domain.device_token;


int main()
{
    using namespace will::domain;

    {
        const auto parsed = DeviceToken::parse("abcd1234abcd1234abcd1234abcd1234");
        assert(parsed);
        assert(parsed->text() == "abcd1234abcd1234abcd1234abcd1234");
    }

    {
        const AuthToken generated = DeviceToken::generate();
        assert(generated.value.size() == 32);
        assert(DeviceToken::parse(generated.value));
    }

    assert(!DeviceToken::parse(""));
    assert(!DeviceToken::parse("short"));
    assert(!DeviceToken::parse("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"));

    return EXIT_SUCCESS;
}
