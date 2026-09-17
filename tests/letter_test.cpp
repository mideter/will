#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "acts/obedience.h"
#include "beings/letter.h"
#include "beings/soul.h"
#include "beings/testament.h"
#include "identity/letter.h"
#include "identity/obedience.h"
#include "identity/place.h"
#include "identity/testament.h"
#include "values/soul_name.h"
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
	const Letter letter{id::Letter{1}, id::Place::global(), id::Soul{7}, Word{"hello"}, Timestamp{100}};
	CHECK(letter.id() == id::Letter{1});
	CHECK(letter.place_id() == id::Place::global());
	CHECK(letter.author_id() == id::Soul{7});
	CHECK(letter.body() == "hello");
	CHECK(letter.created_at() == Timestamp{100});
}


TEST_CASE("Letter rejects invalid construction")
{
	CHECK_THROWS_AS(
		(Letter{id::Letter{1}, id::Place::global(), id::Soul{0}, Word{"x"}, Timestamp{0}}),
		std::invalid_argument);
	CHECK_THROWS_AS((Word{""}), std::invalid_argument);
	CHECK_THROWS_AS((Word{std::string(Word::MaxBodyLength + 1, 'a')}), std::invalid_argument);
}


TEST_CASE("Letter accepts max body length")
{
	const Letter max_body{id::Letter{1}, id::Place::global(), id::Soul{1},
						  Word{std::string(Word::MaxBodyLength, 'a')}, Timestamp{0}};
	CHECK(max_body.body().size() == Word::MaxBodyLength);
}


TEST_CASE("id::Testament requires positive value")
{
	CHECK(id::Testament{4}.value() == 4);
	CHECK_THROWS_AS(id::Testament{0}, std::invalid_argument);
}


TEST_CASE("Testament is a letter in an obedience place")
{
	const id::Testament tid{3};
	const id::Obedience oid{9};
	const Soul testator{id::Soul{1}, *SoulName::parse("testator")};
	const Soul executor{id::Soul{2}, *SoulName::parse("executor")};
	const Obedience obedience{oid, testator, executor};

	const Testament testament{tid, obedience, Word{"do this"}, Timestamp{50}};
	CHECK(testament.id() == id::Letter{tid.value()});
	CHECK(testament.obedience_id() == oid);
	CHECK(testament.place_id() == id::Place{oid.value()});
	CHECK(&testament.testator() == &testator);
	CHECK(testament.author_id() == testator.id());
	CHECK(&testament.executor() == &executor);
	CHECK(testament.body() == "do this");
	CHECK(testament.open());
	CHECK_FALSE(testament.executed());
	CHECK_FALSE(testament.cancelled());

	CHECK_THROWS_AS((Testament{id::Testament{6}, obedience, Word{"x"}, Timestamp{1}, Timestamp{2},
							   Timestamp{3}}),
					std::invalid_argument);
}
