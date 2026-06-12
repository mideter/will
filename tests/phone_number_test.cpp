#include "support/phone_number.h"

#include <cassert>
#include <cstdlib>


int main()
{
    using namespace will::domain;

    {
        const auto parsed = PhoneNumber::parse("+15551234567");
        assert(parsed);
        assert(parsed->e164() == "+15551234567");
    }

    {
        const auto parsed = PhoneNumber::parse("+1 (555) 123-4567");
        assert(parsed);
        assert(parsed->e164() == "+15551234567");
    }

    {
        const auto parsed = PhoneNumber::parse("0015551234567");
        assert(parsed);
        assert(parsed->e164() == "+15551234567");
    }

    {
        const auto parsed = PhoneNumber::parse("15551234567");
        assert(parsed);
        assert(parsed->e164() == "+15551234567");
    }

    assert(!PhoneNumber::parse(""));
    assert(!PhoneNumber::parse("+"));
    assert(!PhoneNumber::parse("+0"));
    assert(!PhoneNumber::parse("abc"));
    assert(!PhoneNumber::parse("+1555"));
    assert(!PhoneNumber::parse("+1234567890123456"));

    {
        const auto a = PhoneNumber::parse("+447700900123");
        const auto b = PhoneNumber::parse("+44 7700 900123");
        assert(a && b);
        assert(*a == *b);
    }

    return EXIT_SUCCESS;
}
