#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "values/abode_name.h"

#include <stdexcept>
#include <string>


using namespace will::domain;


TEST_CASE("AbodeName ctor accepts valid names")
{
	CHECK(AbodeName{"a"} == "a");
	CHECK(AbodeName{"world"} == "world");
	CHECK(AbodeName{"abode01"} == "abode01");
	CHECK(AbodeName{std::string(AbodeName::MaxLength, 'x')}.text().size() == AbodeName::MaxLength);
}


TEST_CASE("AbodeName ctor rejects invalid names")
{
	CHECK_THROWS_AS(AbodeName{""}, std::invalid_argument);
	CHECK_THROWS_AS(AbodeName{"World"}, std::invalid_argument);
	CHECK_THROWS_AS(AbodeName{"abode-1"}, std::invalid_argument);
	CHECK_THROWS_AS(AbodeName{std::string(AbodeName::MaxLength + 1, 'x')}, std::invalid_argument);
}
