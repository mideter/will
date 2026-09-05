#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "values/device_token.h"


using namespace will::domain;


TEST_CASE("DeviceToken::parse accepts valid hex")
{
	const auto parsed = DeviceToken::parse("abcd1234abcd1234abcd1234abcd1234");
	REQUIRE(parsed);
	CHECK(parsed->text() == "abcd1234abcd1234abcd1234abcd1234");
	CHECK(*parsed == *DeviceToken::parse("abcd1234abcd1234abcd1234abcd1234"));
}


TEST_CASE("DeviceToken::generate produces a parseable 32-char token")
{
	const DeviceToken generated = DeviceToken::generate();
	CHECK(generated.text().size() == 32);
	CHECK(DeviceToken::parse(generated.text()));
}


TEST_CASE("DeviceToken::parse rejects invalid tokens")
{
	CHECK_FALSE(DeviceToken::parse(""));
	CHECK_FALSE(DeviceToken::parse("short"));
	CHECK_FALSE(DeviceToken::parse("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"));
}
