#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "identity/abode.h"
#include "identity/letter.h"
#include "identity/man.h"
#include "identity/soul.h"
#include "identity/vessel.h"

#include <stdexcept>


using namespace will::domain;


TEST_CASE("id::Soul requires positive value")
{
	CHECK(id::Soul{42}.value() == 42);
	CHECK_THROWS_AS(id::Soul{0}, std::invalid_argument);
}


TEST_CASE("id::Letter requires positive value")
{
	CHECK(id::Letter{7}.value() == 7);
	CHECK_THROWS_AS(id::Letter{0}, std::invalid_argument);
}


TEST_CASE("id::Abode global and positive values")
{
	CHECK(id::Abode::global() == id::Abode{1});
	CHECK(id::Abode{5}.value() == 5);
	CHECK(id::Abode{5} != id::Abode::global());
	CHECK_THROWS_AS(id::Abode{0}, std::invalid_argument);
}


TEST_CASE("id::Vessel and id::Man require positive values")
{
	CHECK(id::Man{3}.value() == 3);
	CHECK_THROWS_AS(id::Vessel{0}, std::invalid_argument);
	CHECK_THROWS_AS(id::Man{0}, std::invalid_argument);
}
