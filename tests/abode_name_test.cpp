#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "values/abode_name.h"


using namespace will::domain;


TEST_CASE("AbodeName::parse accepts valid names")
{
	CHECK(AbodeName::parse("a"));
	CHECK(AbodeName::parse("world"));
	CHECK(AbodeName::parse("abode01"));
	CHECK(AbodeName::parse(std::string(AbodeName::MaxLength, 'x')));
}


TEST_CASE("AbodeName::parse rejects invalid names")
{
	CHECK_FALSE(AbodeName::parse(""));
	CHECK_FALSE(AbodeName::parse("World"));
	CHECK_FALSE(AbodeName::parse("abode-1"));
	CHECK_FALSE(AbodeName::parse(std::string(AbodeName::MaxLength + 1, 'x')));
}


TEST_CASE("AbodeName::global is parseable world")
{
	CHECK(AbodeName::global().text() == "world");
	CHECK(AbodeName::parse(AbodeName::global().text()));
}
