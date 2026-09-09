#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "entities/letter.h"
#include "identity/letter.h"
#include "values/word.h"

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
	const Letter letter{id::Letter{1}, id::Abode::global(), id::Soul{7}, Word{"hello"}, Timestamp{100}};
	CHECK(letter.id() == id::Letter{1});
	CHECK(letter.abode_id() == id::Abode::global());
	CHECK(letter.author_id() == id::Soul{7});
	CHECK(letter.body() == "hello");
	CHECK(letter.created_at() == Timestamp{100});
}


TEST_CASE("Letter rejects invalid construction")
{
	CHECK_THROWS_AS(
		(Letter{id::Letter{1}, id::Abode::global(), id::Soul{0}, Word{"x"}, Timestamp{0}}),
		std::invalid_argument);
	CHECK_THROWS_AS((Word{""}), std::invalid_argument);
	CHECK_THROWS_AS((Word{std::string(Word::MaxBodyLength + 1, 'a')}), std::invalid_argument);
}


TEST_CASE("Letter accepts max body length")
{
	const Letter max_body{id::Letter{1}, id::Abode::global(), id::Soul{1},
						  Word{std::string(Word::MaxBodyLength, 'a')}, Timestamp{0}};
	CHECK(max_body.body().size() == Word::MaxBodyLength);
}
