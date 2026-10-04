#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "domain_fakes.h"

#include "acts/creation.h"
#include "matter/dating.h"
#include "matter/behest.h"
#include "matter/deed.h"
#include "matter/letter.h"
#include "matter/execution.h"
#include "matter/placement.h"
#include "immanents/tie.h"
#include "matter/tie.h"
#include "matter/word.h"
#include "words/letter.h"
#include "words/behest.h"
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

	const auto letters = letters_seen_by(world.contemplation(man.Soul::id()));
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


TEST_CASE("Behest is a word of will in a living Tie, born of the Tie or of its testator")
{
	InMemoryCosmos cosmos;
	Creation creation = cosmos.life().create();
	World& world = creation.world();

	const auto& testator = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));
	const auto& novice = static_cast<const Novice&>(world.welcome(DeviceToken::generate()));

	static_assert(!std::is_constructible_v<Behest, matter::Behest>);

	novice.supplicate(testator);
	const Shepherding& shepherding = testator.accept(*testator.supplication(novice));

	cosmos.fake_time().set_instant(Timestamp{50});
	const Behest behest = testator.will(shepherding, "do this");
	CHECK(&behest.tie().testator() == &testator);
	CHECK(&behest.tie().novice() == &novice);
	CHECK(behest.saying().body() == "do this");
	CHECK(behest.created_at() == Timestamp{50});

	const auto shown = novice.obedience(testator).behests(novice);
	REQUIRE(shown.size() == 1);
	CHECK(shown.front()->id() == behest.id());

	const matter::Word word{behest.id(), testator.Soul::id(), "do this"};
	const matter::Dating dating{behest.id(), Timestamp{50}};
	CHECK_THROWS_AS((matter::Behest{word, matter::Placement{id::Word{4}, shown.front()->tie().id()}, dating}),
					std::invalid_argument);
}


TEST_CASE("Deed is the novice's word fulfilling a behest, his report or «совершено»")
{
	InMemoryCosmos cosmos;
	Creation creation = cosmos.life().create();
	World& world = creation.world();

	const auto& testator = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));
	const auto& novice = static_cast<const Novice&>(world.welcome(DeviceToken::generate()));

	static_assert(!std::is_constructible_v<Deed, matter::Deed>);

	novice.supplicate(testator);
	const Shepherding& shepherding = testator.accept(*testator.supplication(novice));
	const Behest fast = testator.will(shepherding, "fast");
	const Behest pray = testator.will(shepherding, "pray");

	// Only the novice of the tie fulfils its behest.
	CHECK_THROWS_AS(testator.execute(fast), std::logic_error);

	cosmos.fake_time().set_instant(Timestamp{70});
	const std::shared_ptr<const Deed> silent = novice.execute(fast);
	CHECK(silent->behest() == fast.id());
	CHECK(silent->saying().body() == "совершено");
	CHECK(&silent->tie().novice() == &novice);
	CHECK(silent->created_at() == Timestamp{70});
	CHECK(silent->id() != fast.id());

	const std::shared_ptr<const Deed> reported = novice.execute(pray, Saying{"prayed till dawn"});
	CHECK(reported->behest() == pray.id());
	CHECK(reported->saying().body() == "prayed till dawn");

	// A behest is fulfilled once; the behest itself does not change.
	CHECK_THROWS_AS(novice.execute(fast), std::logic_error);
	CHECK(fast.saying().body() == "fast");

	// The tie shows its behests and, apart, the deeds that fulfil them.
	CHECK(novice.obedience(testator).behests(novice).size() == 2);
	const auto deeds = shepherding.deeds(testator);
	REQUIRE(deeds.size() == 2);
	CHECK(deeds[0]->behest() == fast.id());
	CHECK(deeds[1]->behest() == pray.id());

	// The parts of a deed share its word id; the execution names another word.
	const matter::Word word{id::Word{90}, novice.Soul::id(), "x"};
	const matter::Placement placement{id::Word{90}, shepherding.id()};
	const matter::Dating dating{id::Word{90}, Timestamp{1}};
	CHECK_THROWS_AS((matter::Deed{word, placement, dating, matter::Execution{id::Word{91}, fast.id()}}),
					std::invalid_argument);
	CHECK_THROWS_AS((matter::Execution{id::Word{90}, id::Word{90}}), std::invalid_argument);
}
