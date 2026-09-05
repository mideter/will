#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "values/soul_name.h"


using namespace will::domain;


TEST_CASE("SoulName::parse accepts valid names")
{
	CHECK(SoulName::parse("abcdefgh"));
	CHECK(SoulName::parse("abc12345"));
}


TEST_CASE("SoulName::parse rejects invalid names")
{
	CHECK_FALSE(SoulName::parse(""));
	CHECK_FALSE(SoulName::parse("short"));
	CHECK_FALSE(SoulName::parse("ABCDEFGH"));
	CHECK_FALSE(SoulName::parse("abcd-efg"));
}


TEST_CASE("SoulName::generate produces a parseable name")
{
	const SoulName name = SoulName::generate();
	CHECK(SoulName::parse(name.text()));
}
