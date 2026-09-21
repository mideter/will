#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "domain_fakes.h"

#include "acts/creation.h"
#include "acts/tie.h"
#include "acts/tying.h"
#include "beings/letter.h"
#include "beings/deed.h"
#include "beings/novice.h"
#include "beings/testator.h"
#include "identity/letter.h"
#include "identity/tie.h"
#include "identity/place.h"
#include "identity/deed.h"
#include "values/word.h"

#include <stdexcept>
#include <string>


using namespace will::domain;
using namespace will::domain::test;


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


TEST_CASE("id::Deed requires positive value")
{
	CHECK(id::Deed{4}.value() == 4);
	CHECK_THROWS_AS(id::Deed{0}, std::invalid_argument);
}


TEST_CASE("Deed is a letter in an obedience place")
{
	InMemoryTemporality temporality;
	Creation creation(temporality);
	World& world = creation.world();

	const auto& testator = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));
	const auto& novice = static_cast<const Novice&>(world.welcome(DeviceToken::generate()));
	const id::Deed did{3};
	const id::Tie oid{9};
	const Tie tie{Tying{oid, testator.Soul::id(), novice.Soul::id()}};

	const Deed deed{did, tie, Word{"do this"}, Timestamp{50}};
	CHECK(deed.id() == id::Letter{did.value()});
	CHECK(&deed.tie() == &tie);
	CHECK(deed.place_id() == id::Place{oid.value()});
	CHECK(&deed.tie().testator() == &testator);
	CHECK(deed.author_id() == testator.Soul::id());
	CHECK(&deed.tie().novice() == &novice);
	CHECK(deed.body() == "do this");
	CHECK(deed.open());
	CHECK_FALSE(deed.executed());
	CHECK_FALSE(deed.cancelled());

	CHECK_THROWS_AS((Deed{id::Deed{6}, tie, Word{"x"}, Timestamp{1}, Timestamp{2}, Timestamp{3}}),
					std::invalid_argument);
}
