#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "values/timestamp.h"

#include <stdexcept>


using namespace will::domain;


TEST_CASE("Timestamp stores non-negative nanoseconds")
{
	const Timestamp ts{1000};
	CHECK(ts.value() == 1000);
	CHECK(Timestamp{0}.value() == 0);
}


TEST_CASE("Timestamp rejects negative values")
{
	CHECK_THROWS_AS(Timestamp{-1}, std::invalid_argument);
}


TEST_CASE("default Timestamp is wall-clock now")
{
	CHECK(Timestamp{}.value() >= 1'000'000'000'000'000'000LL);
}
