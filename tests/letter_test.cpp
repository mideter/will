#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "entities/letter.h"
#include "identity/letter.h"

#include <stdexcept>
#include <string>


using namespace will::domain;


TEST_CASE("id::Letter requires positive value")
{
	CHECK(id::Letter{1}.value() == 1);
	CHECK_THROWS_AS(id::Letter{0}, std::invalid_argument);
}


TEST_CASE("Letter stores fields")
{
	const Letter letter{id::Letter{1}, id::Abode::global(), id::Soul{7}, "hello", Timestamp{100}};
	CHECK(letter.id() == id::Letter{1});
	CHECK(letter.abode_id() == id::Abode::global());
	CHECK(letter.author_id() == id::Soul{7});
	CHECK(letter.body() == "hello");
	CHECK(letter.created_at() == Timestamp{100});
}


TEST_CASE("Letter rejects invalid construction")
{
	CHECK_THROWS_AS(
		(Letter{id::Letter{1}, id::Abode::global(), id::Soul{0}, "x", Timestamp{0}}),
		std::invalid_argument);
	CHECK_THROWS_AS(
		(Letter{id::Letter{1}, id::Abode::global(), id::Soul{1}, "", Timestamp{0}}),
		std::invalid_argument);
	CHECK_THROWS_AS(
		(Letter{id::Letter{1}, id::Abode::global(), id::Soul{1},
			std::string(Letter::MaxBodyLength + 1, 'a'), Timestamp{0}}),
		std::invalid_argument);
}


TEST_CASE("Letter accepts max body length")
{
	const Letter max_body{id::Letter{1}, id::Abode::global(), id::Soul{1},
		std::string(Letter::MaxBodyLength, 'a'), Timestamp{0}};
	CHECK(max_body.body().size() == Letter::MaxBodyLength);
}
