#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "domain_fakes.h"

#include "acts/creation.h"
#include "beings/immanents/obedience.h"
#include "beings/immanents/shepherding.h"
#include "beings/immanents/tie.h"
#include "acts/tying.h"
#include "beings/deed.h"
#include "beings/immanents/novice.h"
#include "beings/immanents/testator.h"
#include "beings/immanents/witness.h"
#include "values/abode_name.h"
#include "values/device_token.h"
#include "values/timestamp.h"
#include "values/soul_name.h"

#include <stdexcept>
#include <vector>


namespace {


using namespace will::domain;
using namespace will::domain::test;


DeviceToken test_token(const char* hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
{
	return *DeviceToken::parse(hex);
}


SoulName test_name(const char* text)
{
	return *SoulName::parse(text);
}


} // namespace


TEST_CASE("welcome creates man with personal abode")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const DeviceToken token = DeviceToken::generate();
	const Man& man = world.welcome(token);
	const auto& witness = static_cast<const Witness&>(man);

	CHECK(man.Soul::id().value() > 0);
	CHECK(man.Vessel::id().value() > 0);
	CHECK(witness.abode().id().value() > 0);
	CHECK(witness.abode().abode_id() == id::Abode{witness.abode().id()});
	CHECK(witness.abode().dwells(man));
	CHECK(&world.contemplation(man.Soul::id()).abode() == &man.abode());
	CHECK(world.contemplating(man.abode()).size() == 1);
	CHECK(&world.contemplating(man.abode()).front().get() == static_cast<const Soul*>(&man));
	CHECK(world.knows(man.Vessel::id()));
	CHECK(world.knows(man.Soul::id()));

	const Vessel& vessel = world.vessel(man.Vessel::id());
	CHECK(vessel.id() == man.Vessel::id());
	CHECK(world.man(vessel).Soul::id() == man.Soul::id());
	CHECK(&world.man(vessel) == &man);
	CHECK(world.man(vessel).Soul::id() == man.Soul::id());

	const Soul& soul = world.soul(man.Soul::id());
	CHECK(&soul == static_cast<const Soul*>(&man));
	CHECK(SoulName::parse(soul.name().text()));
	CHECK(dynamic_cast<const Testator*>(&man));
	CHECK(dynamic_cast<const Novice*>(&man));
	CHECK(dynamic_cast<const Witness*>(&man));
}


TEST_CASE("welcome existing man")
{
	InMemoryCosmos cosmos;
	seed_man(cosmos.temporality(), id::Soul{42}, test_token("abcd1234abcd1234abcd1234abcd1234"), test_name("oldname1"));
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& man = world.welcome(test_token("abcd1234abcd1234abcd1234abcd1234"));
	CHECK(man.Soul::id() == id::Soul{42});
	CHECK(static_cast<const Witness&>(man).abode().dwells(man));
	CHECK(static_cast<const Witness&>(man).abode().id().value() > 0);
	CHECK(static_cast<const Witness&>(man).abode().id() != id::Place{man.Soul::id().value()});
}


TEST_CASE("welcome keeps existing name")
{
	InMemoryCosmos cosmos;
	seed_man(cosmos.temporality(), id::Soul{7}, test_token("abcd1234abcd1234abcd1234abcd1234"), test_name("keptname"));
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	(void)world.welcome(test_token("abcd1234abcd1234abcd1234abcd1234"));

	CHECK(world.soul(id::Soul{7}).name() == test_name("keptname"));
}


TEST_CASE("each man has a distinct personal abode")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& a = world.welcome(DeviceToken::generate());
	const Man& b = world.welcome(DeviceToken::generate());
	const auto& wa = static_cast<const Witness&>(a);
	const auto& wb = static_cast<const Witness&>(b);

	CHECK(wa.abode().id() != wb.abode().id());
	CHECK(&wa.abode() != &wb.abode());
	CHECK(wa.abode().dwells(a));
	CHECK_FALSE(wa.abode().dwells(b));
	CHECK(wb.abode().dwells(b));
	CHECK_FALSE(wb.abode().dwells(a));
	CHECK(&world.contemplation(a.Soul::id()).abode() == &a.abode());
	CHECK(&world.contemplation(b.Soul::id()).abode() == &b.abode());
	CHECK(world.contemplating(a.abode()).size() == 1);
	CHECK(world.contemplating(b.abode()).size() == 1);
	CHECK(&world.contemplation(a.Soul::id()).abode() != &b.abode());
	CHECK(&world.contemplating(a.abode()).front().get() == static_cast<const Soul*>(&a));
	CHECK(&world.contemplating(b.abode()).front().get() == static_cast<const Soul*>(&b));
}


TEST_CASE("man say persists via temporality")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& author = world.welcome(DeviceToken::generate());
	const auto& witness = static_cast<const Witness&>(author);
	author.say("hello");

	const auto placed = cosmos.spatiality().placements(witness.abode().id(), 10);
	REQUIRE(placed.size() == 1);
	const auto dated = cosmos.temporality().datings({placed[0].id()});
	REQUIRE(dated.size() == 1);
	const auto uttered = cosmos.eternity().utterance(placed[0].id());
	CHECK(uttered.author() == author.Soul::id());
	CHECK(uttered.saying().body() == "hello");
	CHECK(dated[0].created_at() == Timestamp{1});
}


TEST_CASE("witness retell is history of observed abode")
{
	InMemoryCosmos cosmos;
	const id::Soul author{1};

	seed_man(cosmos.temporality(), author, test_token("feedfacefeedfacefeedfacefeedface"), test_name("authoraa"));
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& man = world.welcome(test_token("feedfacefeedfacefeedfacefeedface"));
	const auto& witness = static_cast<const Witness&>(man);

	for (int i = 0; i < 5; ++i)
		man.say("m");

	CHECK_THROWS_AS(witness.retell(0), std::invalid_argument);

	const auto items = witness.retell(Spatiality::MaxLetterLimit + 50);
	CHECK(items.size() == 5);
}


TEST_CASE("tie is a place for distinct testator and novice")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const auto& testator = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));
	const auto& novice = static_cast<const Novice&>(world.welcome(DeviceToken::generate()));
	const id::Tie oid{7};

	const Tie tie{Tying{oid, testator.Soul::id(), novice.Soul::id()}};
	CHECK(tie.id() == id::Place{oid.value()});
	CHECK(&tie.testator() == &testator);
	CHECK(&tie.novice() == &novice);

	CHECK_THROWS_AS((Tying{id::Tie{8}, testator.Soul::id(), testator.Soul::id()}),
					std::invalid_argument);
}


TEST_CASE("supplicate accept creates tie owned as obedience")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& a = world.welcome(DeviceToken::generate());
	const Man& b = world.welcome(DeviceToken::generate());
	const auto& novice = static_cast<const Novice&>(a);
	const auto& testator = static_cast<const Testator&>(b);

	novice.supplicate(testator);
	CHECK(testator.supplications().size() == 1);
	const Supplication& ask = testator.supplication(novice);
	CHECK(ask.suppliant().Soul::id() == a.Soul::id());
	CHECK(ask.addressee().Soul::id() == b.Soul::id());
	CHECK(cosmos.temporality().pending_supplications(b.Soul::id()).size() == 1);

	testator.accept(ask);
	CHECK(testator.supplications().empty());
	REQUIRE(cosmos.temporality().tyings().size() == 1);
	const Obedience& obedience =
		novice.obedience(cosmos.temporality().tyings().front().id());
	CHECK(&obedience.testator() == &testator);
	CHECK(&obedience.novice() == &novice);
	CHECK(obedience.id().value() != a.Soul::id().value());
	CHECK(obedience.id().value() != b.Soul::id().value());
	CHECK(cosmos.temporality().pending_supplications(b.Soul::id()).empty());
	CHECK(&novice.obedience(id::Tie{obedience.id()}) == &obedience);
	CHECK(&testator.shepherding(id::Tie{obedience.id()}).testator() == &testator);
}


TEST_CASE("reject closes pending supplication; wrong party cannot accept")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& a = world.welcome(DeviceToken::generate());
	const Man& b = world.welcome(DeviceToken::generate());
	const auto& novice = static_cast<const Novice&>(a);
	const auto& testator = static_cast<const Testator&>(b);
	const auto& stranger = static_cast<const Testator&>(a);

	novice.supplicate(testator);
	const Supplication& ask = testator.supplication(novice);
	CHECK_THROWS_AS(stranger.accept(ask), std::logic_error);

	testator.reject(ask);
	CHECK(testator.supplications().empty());
	CHECK(cosmos.temporality().pending_supplications(b.Soul::id()).empty());
	CHECK_THROWS_AS(testator.supplication(novice), std::invalid_argument);
}


TEST_CASE("will and execute within obedience")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& a = world.welcome(DeviceToken::generate());
	const Man& b = world.welcome(DeviceToken::generate());
	CHECK(&world.man(a.name()) == &a);
	CHECK(&world.man(b.name()) == &b);
	CHECK_THROWS_AS(world.man(*SoulName::parse("zzzzzzzz")), std::invalid_argument);

	const auto& novice = static_cast<const Novice&>(a);
	const auto& testator = static_cast<const Testator&>(b);

	novice.supplicate(testator);
	testator.accept(testator.supplication(novice));
	REQUIRE(cosmos.temporality().tyings().size() == 1);
	const Obedience& obedience =
		novice.obedience(cosmos.temporality().tyings().front().id());
	const Shepherding& shepherding = testator.shepherding(id::Tie{obedience.id()});
	const Deed deed = testator.will(shepherding, "fast");
	CHECK(deed.open());
	CHECK(deed.saying().body() == "fast");
	CHECK(&deed.tie().testator() == &testator);
	CHECK(&deed.tie().novice() == &novice);

	CHECK_THROWS_AS(static_cast<const Testator&>(a).will(shepherding, "no"), std::logic_error);

	CHECK(novice.deed(deed.id()).open());
	CHECK(testator.deed(deed.id()).id() == deed.id());
	CHECK_THROWS_AS(testator.execute(deed), std::logic_error);

	const Deed done = novice.execute(deed);
	CHECK(done.executed());
	CHECK_FALSE(done.open());
	CHECK(novice.deed(deed.id()).executed());
	CHECK_THROWS_AS(novice.execute(done), std::logic_error);

	// Stale open snapshot: what is kept decides, not the argument.
	CHECK_THROWS_AS(novice.execute(deed), std::logic_error);
	CHECK(cosmos.temporality().executions({deed.id()}).size() == 1);
}


TEST_CASE("deed is unknown to a soul outside its tie and is not a letter")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const auto& novice = static_cast<const Novice&>(world.welcome(DeviceToken::generate()));
	const auto& testator = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));
	const auto& stranger = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));

	novice.supplicate(testator);
	testator.accept(testator.supplication(novice));
	const id::Tie tie = cosmos.temporality().tyings().front().id();
	const Deed deed = testator.will(testator.shepherding(tie), "fast");

	CHECK_THROWS_AS(stranger.deed(deed.id()), std::invalid_argument);
	CHECK_THROWS_AS(novice.deed(id::Word{999}), std::invalid_argument);

	// A deed lives in its tie, not in the abodes its sides contemplate.
	CHECK(testator.retell(10).empty());
	CHECK(novice.retell(10).empty());

	// A letter is not a deed.
	static_cast<const Witness&>(novice).say("hello");
	const id::Word letter = novice.retell(10).front().id();
	CHECK_THROWS_AS(novice.deed(letter), std::invalid_argument);
}
