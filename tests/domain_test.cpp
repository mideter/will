#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "domain_fakes.h"

#include "entities/world.h"
#include "entities/witness.h"
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


TEST_CASE("welcome creates man")
{
	InMemoryTemporality temporality;
	World world(temporality);

	CHECK(world.abode().name() == AbodeName::global());

	const DeviceToken token = DeviceToken::generate();
	const Man& man = world.welcome(token);

	CHECK(man.Soul::id().value() > 0);
	CHECK(man.id().value() > 0);
	CHECK(man.Vessel::id().value() > 0);
	CHECK(world.abode().dwells(man));
	CHECK(&static_cast<const Witness&>(man).abode() == &world.abode());
	CHECK(world.knows(man.Vessel::id()));
	CHECK(world.knows(man.Soul::id()));

	const Vessel& vessel = world.vessel(man.Vessel::id());
	CHECK(vessel.id() == man.Vessel::id());
	CHECK(world.man(vessel).id() == man.id());
	CHECK(&world.man(vessel) == &man);
	CHECK(world.man(vessel).Soul::id() == man.Soul::id());

	const Soul& soul = world.soul(man.Soul::id());
	CHECK(&soul == static_cast<const Soul*>(&man));
	CHECK(SoulName::parse(soul.name().text()));
}


TEST_CASE("welcome existing man")
{
	InMemoryTemporality temporality;
	seed_man(temporality, id::Soul{42}, test_token("abcd1234abcd1234abcd1234abcd1234"), test_name("oldname1"));
	World world(temporality);

	const Man& man = world.welcome(test_token("abcd1234abcd1234abcd1234abcd1234"));
	CHECK(man.Soul::id() == id::Soul{42});
	CHECK(world.abode().dwells(man));
	CHECK(&static_cast<const Witness&>(man).abode() == &world.abode());
}


TEST_CASE("welcome keeps existing name")
{
	InMemoryTemporality temporality;
	seed_man(temporality, id::Soul{7}, test_token("abcd1234abcd1234abcd1234abcd1234"), test_name("keptname"));
	World world(temporality);

	(void)world.welcome(test_token("abcd1234abcd1234abcd1234abcd1234"));

	CHECK(world.soul(id::Soul{7}).name() == test_name("keptname"));
}


TEST_CASE("man say persists via temporality")
{
	InMemoryTemporality temporality;
	World world(temporality);

	const Man& author = world.welcome(DeviceToken::generate());
	author.say(Word{"hello"});

	const auto loaded = temporality.letters(world.abode().id(), 10);
	REQUIRE(loaded.size() == 1);
	CHECK(loaded[0].author_id() == author.Soul::id());
	CHECK(loaded[0].body() == "hello");
	CHECK(loaded[0].created_at() == Timestamp{1});
}


TEST_CASE("witness hear returns latest word")
{
	InMemoryTemporality temporality;
	const id::Soul me{10};
	const id::Soul other{20};

	seed_man(temporality, me, test_token("c0ffee00c0ffee00c0ffee00c0ffee00"), test_name("menameaa"));
	seed_man(temporality, other, test_token("deadbeefdeadbeefdeadbeefdeadbeef"), test_name("peername"));
	World world(temporality);

	const Man& listener = world.welcome(test_token("c0ffee00c0ffee00c0ffee00c0ffee00"));

	CHECK_THROWS_AS(listener.hear(), std::logic_error);

	temporality.fix(world.abode().id(), other, Word{"peer"});
	temporality.fix(world.abode().id(), me, Word{"mine"});

	CHECK(listener.hear().body() == "mine");
}


TEST_CASE("world letters is history through port")
{
	InMemoryTemporality temporality;
	const id::Soul author{1};

	seed_man(temporality, author, test_token("feedfacefeedfacefeedfacefeedface"), test_name("authoraa"));
	World world(temporality);

	for (int i = 0; i < 5; ++i)
		temporality.fix(world.abode().id(), author, Word{"m"});

	CHECK_THROWS_AS(world.letters(0), std::invalid_argument);

	const auto items = world.letters(Temporality::MaxLetterLimit + 50);
	CHECK(items.size() == 5);
}
