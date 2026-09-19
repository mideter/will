#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "identity/abode.h"
#include "identity/letter.h"
#include "identity/obedience.h"
#include "identity/place.h"
#include "identity/soul.h"
#include "identity/supplication.h"
#include "identity/deed.h"
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


TEST_CASE("id::Place global and positive values")
{
	CHECK(id::Place::global() == id::Place{1});
	CHECK(id::Place{5}.value() == 5);
	CHECK_THROWS_AS(id::Place{0}, std::invalid_argument);
}


TEST_CASE("id::Obedience requires positive value")
{
	CHECK(id::Obedience{9}.value() == 9);
	CHECK_THROWS_AS(id::Obedience{0}, std::invalid_argument);
}


TEST_CASE("id::Supplication requires positive value")
{
	CHECK(id::Supplication{3}.value() == 3);
	CHECK_THROWS_AS(id::Supplication{0}, std::invalid_argument);
}


TEST_CASE("id::Deed requires positive value")
{
	CHECK(id::Deed{4}.value() == 4);
	CHECK_THROWS_AS(id::Deed{0}, std::invalid_argument);
}


TEST_CASE("id::Vessel requires positive value")
{
	CHECK(id::Vessel{3}.value() == 3);
	CHECK_THROWS_AS(id::Vessel{0}, std::invalid_argument);
}
