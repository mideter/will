#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "domain_fakes.h"

#include "acts/creation.h"
#include "acts/inscription.h"
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


TEST_CASE("Inscription stores fields")
{
	const Inscription inscription{id::Letter{1}, id::Place::global(), id::Soul{7}, Word{"hello"},
						Timestamp{100}};
	CHECK(inscription.id() == id::Letter{1});
	CHECK(inscription.place() == id::Place::global());
	CHECK(inscription.author() == id::Soul{7});
	CHECK(inscription.word().body() == "hello");
	CHECK(inscription.created_at() == Timestamp{100});
}


TEST_CASE("Inscription rejects invalid construction")
{
	CHECK_THROWS_AS(
		(Inscription{id::Letter{1}, id::Place::global(), id::Soul{0}, Word{"x"}, Timestamp{0}}),
		std::invalid_argument);
	CHECK_THROWS_AS((Word{""}), std::invalid_argument);
	CHECK_THROWS_AS((Word{std::string(Word::MaxBodyLength + 1, 'a')}), std::invalid_argument);
}


TEST_CASE("Inscription accepts max body length")
{
	const Inscription max_body{id::Letter{1}, id::Place::global(), id::Soul{1},
						  Word{std::string(Word::MaxBodyLength, 'a')}, Timestamp{0}};
	CHECK(max_body.word().body().size() == Word::MaxBodyLength);
}


TEST_CASE("Letter is born from Inscription with living place and author")
{
	InMemoryTemporality temporality;
	Creation creation(temporality);
	World& world = creation.world();

	const Man& man = world.welcome(DeviceToken::generate());
	const auto& witness = static_cast<const Witness&>(man);

	const Inscription inscription{id::Letter{1}, witness.abode().id(), man.Soul::id(), Word{"hello"},
						Timestamp{100}};
	const Letter letter{inscription, witness.abode()};

	CHECK(letter.id() == id::Letter{1});
	CHECK(&letter.place() == &witness.abode());
	CHECK(&letter.author() == &static_cast<const Soul&>(man));
	CHECK(letter.body() == "hello");
	CHECK(letter.created_at() == Timestamp{100});
}


TEST_CASE("Letter rejects place mismatch")
{
	InMemoryTemporality temporality;
	Creation creation(temporality);
	World& world = creation.world();

	const Man& a = world.welcome(DeviceToken::generate());
	const Man& b = world.welcome(DeviceToken::generate());
	const auto& wa = static_cast<const Witness&>(a);
	const auto& wb = static_cast<const Witness&>(b);

	const Inscription inscription{id::Letter{1}, wa.abode().id(), a.Soul::id(), Word{"x"}, Timestamp{0}};
	CHECK_THROWS_AS((Letter{inscription, wb.abode()}), std::invalid_argument);
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
	CHECK(&deed.place() == &tie);
	CHECK(&deed.tie().testator() == &testator);
	CHECK(&deed.author() == &testator);
	CHECK(&deed.tie().novice() == &novice);
	CHECK(deed.body() == "do this");
	CHECK(deed.open());
	CHECK_FALSE(deed.executed());
	CHECK_FALSE(deed.cancelled());

	CHECK_THROWS_AS((Deed{id::Deed{6}, tie, Word{"x"}, Timestamp{1}, Timestamp{2}, Timestamp{3}}),
					std::invalid_argument);
}
