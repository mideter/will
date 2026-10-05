#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "identity/word.h"
#include "identity/place.h"
#include "identity/soul.h"
#include "identity/vessel.h"

#include <stdexcept>


using namespace will::domain;


TEST_CASE("id::Soul requires positive value")
{
	CHECK(id::Soul{42}.value() == 42);
	CHECK_THROWS_AS(id::Soul{0}, std::invalid_argument);
}


TEST_CASE("id::Word requires positive value")
{
	CHECK(id::Word{7}.value() == 7);
	CHECK_THROWS_AS(id::Word{0}, std::invalid_argument);
}


TEST_CASE("id::Place requires positive value")
{
	CHECK(id::Place{5}.value() == 5);
	CHECK_THROWS_AS(id::Place{0}, std::invalid_argument);
}


TEST_CASE("id::Vessel requires positive value")
{
	CHECK(id::Vessel{3}.value() == 3);
	CHECK_THROWS_AS(id::Vessel{0}, std::invalid_argument);
}
