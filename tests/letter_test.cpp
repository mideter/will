#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "domain_fakes.h"

#include "acts/creation.h"
#include "matter/dating.h"
#include "matter/deed.h"
#include "matter/letter.h"
#include "matter/execution.h"
#include "matter/placement.h"
#include "immanents/tie.h"
#include "matter/tie.h"
#include "matter/word.h"
#include "words/letter.h"
#include "words/deed.h"
#include "immanents/novice.h"
#include "immanents/obedience.h"
#include "immanents/shepherding.h"
#include "immanents/testator.h"
#include "identity/word.h"
#include "identity/tie.h"
#include "identity/place.h"
#include "values/saying.h"

#include <stdexcept>
#include <type_traits>
#include <string>


using namespace will::domain;
using namespace will::domain::test;


TEST_CASE("id::Word requires positive value")
{
	CHECK(id::Word{1}.value() == 1);
	CHECK_THROWS_AS(id::Word{0}, std::invalid_argument);
}


TEST_CASE("matter::Word stores fields")
{
	const matter::Word word{id::Word{1}, id::Soul{7}, "hello"};
	CHECK(word.id() == id::Word{1});
	CHECK(word.author() == id::Soul{7});
	CHECK(word.saying().body() == "hello");
}


TEST_CASE("matter::Word rejects invalid construction")
{
	CHECK_THROWS_AS((matter::Word{id::Word{1}, id::Soul{0}, "x"}), std::invalid_argument);
	CHECK_THROWS_AS((Saying{""}), std::invalid_argument);
	CHECK_THROWS_AS((Saying{std::string(Saying::MaxBodyLength + 1, 'a')}), std::invalid_argument);
}


TEST_CASE("matter::Word accepts max body length")
{
	const matter::Word max_body{id::Word{1}, id::Soul{1}, std::string(Saying::MaxBodyLength, 'a')};
	CHECK(max_body.saying().body().size() == Saying::MaxBodyLength);
}


TEST_CASE("Letter is born of its abode from matter, with living place and author")
{
	InMemoryCosmos cosmos;
	Creation creation = cosmos.life().create();
	World& world = creation.world();

	const Man& man = world.welcome(DeviceToken::generate());
	const auto& witness = static_cast<const Witness&>(man);

	// Only the abode brings a letter forth.
	static_assert(!std::is_constructible_v<Letter, matter::Letter>);

	cosmos.fake_time().set_instant(Timestamp{100});
	witness.wake();
	man.say("hello");

	const auto letters = world.contemplation(man.Soul::id())->letters();
	REQUIRE(letters.size() == 1);
	const Letter& letter = *letters.front();
	CHECK(&letter.place() == &witness.abode());
	CHECK(&letter.author() == &static_cast<const Soul&>(man));
	CHECK(letter.saying().body() == "hello");
	CHECK(letter.created_at() == Timestamp{100});
}


TEST_CASE("matter::Letter rejects parts with different word ids")
{
	InMemoryCosmos cosmos;
	Creation creation = cosmos.life().create();
	World& world = creation.world();

	const Man& man = world.welcome(DeviceToken::generate());
	const auto& witness = static_cast<const Witness&>(man);

	const matter::Word word{id::Word{1}, man.Soul::id(), "x"};
	const matter::Placement placement{id::Word{2}, witness.abode().id()};
	const matter::Dating dating{id::Word{1}, Timestamp{0}};
	CHECK_THROWS_AS((matter::Letter{word, placement, dating}), std::invalid_argument);
}


TEST_CASE("Deed is a word of will in a living Tie, born of the Tie or of its testator")
{
	InMemoryCosmos cosmos;
	Creation creation = cosmos.life().create();
	World& world = creation.world();

	const auto& testator = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));
	const auto& novice = static_cast<const Novice&>(world.welcome(DeviceToken::generate()));

	static_assert(!std::is_constructible_v<Deed, matter::Deed>);

	novice.supplicate(testator);
	const Shepherding& shepherding = testator.accept(*testator.supplication(novice));

	cosmos.fake_time().set_instant(Timestamp{50});
	const Deed deed = testator.will(shepherding, "do this");
	CHECK(&deed.tie().testator() == &testator);
	CHECK(&deed.tie().novice() == &novice);
	CHECK(deed.saying().body() == "do this");
	CHECK(deed.created_at() == Timestamp{50});
	CHECK(deed.open());
	CHECK_FALSE(deed.executed());

	cosmos.fake_time().set_instant(Timestamp{70});
	novice.execute(deed);
	const auto shown = novice.obedience(testator).deeds(novice);
	REQUIRE(shown.size() == 1);
	CHECK(shown.front()->id() == deed.id());
	CHECK(shown.front()->executed());
	CHECK_FALSE(shown.front()->open());
	CHECK(shown.front()->executed_at() == Timestamp{70});

	const matter::Word word{deed.id(), testator.Soul::id(), "do this"};
	const matter::Placement placement{deed.id(), shown.front()->tie().id()};
	const matter::Dating dating{deed.id(), Timestamp{50}};
	CHECK_THROWS_AS(
		(matter::Deed{word, placement, dating, matter::Execution{id::Word{4}, Timestamp{70}}}),
		std::invalid_argument);
	CHECK_THROWS_AS((matter::Deed{word, matter::Placement{id::Word{4}, shown.front()->tie().id()}, dating}),
					std::invalid_argument);
}
