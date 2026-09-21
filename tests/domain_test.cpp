#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "domain_fakes.h"

#include "acts/creation.h"
#include "acts/obedience.h"
#include "acts/shepherding.h"
#include "beings/deed.h"
#include "beings/novice.h"
#include "beings/testator.h"
#include "beings/witness.h"
#include "identity/deed.h"
#include "values/abode_name.h"
#include "values/device_token.h"
#include "values/timestamp.h"
#include "values/soul_name.h"
#include "values/word.h"

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
	InMemoryTemporality temporality;
	Creation creation(temporality);
	World& world = creation.world();

	const DeviceToken token = DeviceToken::generate();
	const Man& man = world.welcome(token);
	const auto& witness = static_cast<const Witness&>(man);
	const id::Abode own{man.Soul::id().value()};

	CHECK(man.Soul::id().value() > 0);
	CHECK(man.Vessel::id().value() > 0);
	CHECK(witness.abode().id() == id::Place{own.value()});
	CHECK(witness.abode().abode_id() == own);
	CHECK(witness.abode().dwells(man));
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
	InMemoryTemporality temporality;
	seed_man(temporality, id::Soul{42}, test_token("abcd1234abcd1234abcd1234abcd1234"), test_name("oldname1"));
	Creation creation(temporality);
	World& world = creation.world();

	const Man& man = world.welcome(test_token("abcd1234abcd1234abcd1234abcd1234"));
	CHECK(man.Soul::id() == id::Soul{42});
	CHECK(static_cast<const Witness&>(man).abode().dwells(man));
	CHECK(static_cast<const Witness&>(man).abode().abode_id() == id::Abode{man.Soul::id().value()});
	CHECK(static_cast<const Witness&>(man).abode().id() == id::Place{man.Soul::id().value()});
}


TEST_CASE("welcome keeps existing name")
{
	InMemoryTemporality temporality;
	seed_man(temporality, id::Soul{7}, test_token("abcd1234abcd1234abcd1234abcd1234"), test_name("keptname"));
	Creation creation(temporality);
	World& world = creation.world();

	(void)world.welcome(test_token("abcd1234abcd1234abcd1234abcd1234"));

	CHECK(world.soul(id::Soul{7}).name() == test_name("keptname"));
}


TEST_CASE("each man has a distinct personal abode")
{
	InMemoryTemporality temporality;
	Creation creation(temporality);
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
}


TEST_CASE("man say persists via temporality")
{
	InMemoryTemporality temporality;
	Creation creation(temporality);
	World& world = creation.world();

	const Man& author = world.welcome(DeviceToken::generate());
	const auto& witness = static_cast<const Witness&>(author);
	author.say(Word{"hello"});

	const auto loaded = temporality.letters(witness.abode().id(), 10);
	REQUIRE(loaded.size() == 1);
	CHECK(loaded[0].author_id() == author.Soul::id());
	CHECK(loaded[0].body() == "hello");
	CHECK(loaded[0].created_at() == Timestamp{1});
}


TEST_CASE("witness retell is history of observed abode")
{
	InMemoryTemporality temporality;
	const id::Soul author{1};

	seed_man(temporality, author, test_token("feedfacefeedfacefeedfacefeedface"), test_name("authoraa"));
	Creation creation(temporality);
	World& world = creation.world();

	const Man& man = world.welcome(test_token("feedfacefeedfacefeedfacefeedface"));
	const auto& witness = static_cast<const Witness&>(man);

	for (int i = 0; i < 5; ++i)
		temporality.fix(witness.abode().id(), author, Word{"m"});

	CHECK_THROWS_AS(witness.retell(0), std::invalid_argument);

	const auto items = witness.retell(Temporality::MaxLetterLimit + 50);
	CHECK(items.size() == 5);
}


TEST_CASE("obedience is a place for distinct testator and novice")
{
	InMemoryTemporality temporality;
	Creation creation(temporality);
	World& world = creation.world();

	const auto& testator = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));
	const auto& novice = static_cast<const Novice&>(world.welcome(DeviceToken::generate()));
	const id::Obedience oid{7};

	Obedience obedience{oid, testator, novice};
	CHECK(obedience.id() == id::Place{oid.value()});
	CHECK(obedience.obedience_id() == oid);
	CHECK(&obedience.testator() == &testator);
	CHECK(&obedience.novice() == &novice);

	CHECK_THROWS_AS((Obedience{id::Obedience{8}, testator, testator}), std::invalid_argument);
}


TEST_CASE("supplicate accept creates obedience owned on both faces")
{
	InMemoryTemporality temporality;
	Creation creation(temporality);
	World& world = creation.world();

	const Man& a = world.welcome(DeviceToken::generate());
	const Man& b = world.welcome(DeviceToken::generate());
	const auto& novice = static_cast<const Novice&>(a);
	const auto& testator = static_cast<const Testator&>(b);

	novice.supplicate(testator);
	CHECK(testator.supplications().size() == 1);
	const Supplication& ask = testator.supplication(novice);
	CHECK(ask.suppliant().Soul::id() == a.Soul::id());
	CHECK(ask.testator().Soul::id() == b.Soul::id());
	CHECK(temporality.pending_supplications(b.Soul::id()).size() == 1);

	testator.accept(ask);
	CHECK(testator.supplications().empty());
	REQUIRE(temporality.obediences().size() == 1);
	const Obedience& obedience =
		novice.obedience(temporality.obediences().front().obedience_id());
	CHECK(&obedience.testator() == &testator);
	CHECK(&obedience.novice() == &novice);
	CHECK(obedience.id().value() != a.Soul::id().value());
	CHECK(obedience.id().value() != b.Soul::id().value());
	CHECK(temporality.pending_supplications(b.Soul::id()).empty());
	CHECK(&novice.obedience(obedience.obedience_id()) == &obedience);
	CHECK(&testator.shepherding(obedience.obedience_id()).testator() == &testator);
}


TEST_CASE("refuse closes pending supplication; wrong party cannot accept")
{
	InMemoryTemporality temporality;
	Creation creation(temporality);
	World& world = creation.world();

	const Man& a = world.welcome(DeviceToken::generate());
	const Man& b = world.welcome(DeviceToken::generate());
	const auto& novice = static_cast<const Novice&>(a);
	const auto& testator = static_cast<const Testator&>(b);
	const auto& stranger = static_cast<const Testator&>(a);

	novice.supplicate(testator);
	const Supplication& ask = testator.supplication(novice);
	CHECK_THROWS_AS(stranger.accept(ask), std::logic_error);

	testator.refuse(ask);
	CHECK(testator.supplications().empty());
	CHECK(temporality.pending_supplications(b.Soul::id()).empty());
	CHECK_THROWS_AS(testator.supplication(novice), std::invalid_argument);
}


TEST_CASE("will and execute within obedience")
{
	InMemoryTemporality temporality;
	Creation creation(temporality);
	World& world = creation.world();

	const Man& a = world.welcome(DeviceToken::generate());
	const Man& b = world.welcome(DeviceToken::generate());
	const auto& novice = static_cast<const Novice&>(a);
	const auto& testator = static_cast<const Testator&>(b);

	novice.supplicate(testator);
	testator.accept(testator.supplication(novice));
	REQUIRE(temporality.obediences().size() == 1);
	const Obedience& obedience =
		novice.obedience(temporality.obediences().front().obedience_id());
	const Shepherding& shepherding = testator.shepherding(obedience.obedience_id());
	const Deed bequeathed = testator.will(shepherding, Word{"fast"});
	CHECK(bequeathed.open());
	CHECK(bequeathed.body() == "fast");
	CHECK(&bequeathed.testator() == &testator);
	CHECK(&bequeathed.novice() == &novice);

	CHECK_THROWS_AS(static_cast<const Testator&>(a).will(shepherding, Word{"no"}), std::logic_error);

	const Deed done = novice.execute(bequeathed);
	CHECK(done.executed());
	CHECK_FALSE(done.open());
	CHECK_THROWS_AS(novice.execute(done), std::logic_error);
}
