#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "domain_fakes.h"

#include "entities/world.h"
#include "values/abode_name.h"
#include "values/device_token.h"
#include "values/timestamp.h"
#include "values/soul_name.h"
#include "errors/domain_error.h"

#include <variant>
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

	CHECK(man.soul_id().value() > 0);
	CHECK(man.id().value() > 0);
	CHECK(man.vessel_id().value() > 0);
	CHECK(world.abode().shelters(man));
	CHECK(world.knows(token));
	CHECK(world.knows(man.soul_id()));

	const Vessel& vessel = world.vessel(token);
	CHECK(vessel.id() == man.vessel_id());
	CHECK(world.man(vessel).id() == man.id());
	CHECK(&world.man(vessel) == &man);
	CHECK(world.man(vessel).soul_id() == man.soul_id());

	const Soul& soul = world.soul(man.soul_id());
	CHECK(&soul == static_cast<const Soul*>(&man));
	CHECK(SoulName::parse(soul.name().text()));
}


TEST_CASE("welcome existing man")
{
	InMemoryTemporality temporality;
	seed_man(temporality, id::Soul{42}, test_token("abcd1234abcd1234abcd1234abcd1234"), test_name("oldname1"));
	World world(temporality);

	const Man& man = world.welcome(test_token("abcd1234abcd1234abcd1234abcd1234"));
	CHECK(man.soul_id() == id::Soul{42});
	CHECK(world.abode().shelters(man));
}


TEST_CASE("welcome keeps existing name")
{
	InMemoryTemporality temporality;
	seed_man(temporality, id::Soul{7}, test_token("abcd1234abcd1234abcd1234abcd1234"), test_name("keptname"));
	World world(temporality);

	(void)world.welcome(test_token("abcd1234abcd1234abcd1234abcd1234"));

	CHECK(world.soul(id::Soul{7}).name() == test_name("keptname"));
}


TEST_CASE("abode inscribe persists")
{
	InMemoryTemporality temporality;
	World world(temporality);

	const id::Soul author{7};
	const Letter saved = world.abode().inscribe(author, "hello");

	CHECK(saved.id().value() > 0);
	CHECK(saved.author_id() == author);
	CHECK(saved.body() == "hello");
	CHECK(saved.created_at() == Timestamp{1});

	const auto loaded = temporality.letters(world.abode().id(), 10);
	REQUIRE(loaded.size() == 1);
	CHECK(loaded[0].body() == "hello");
}


TEST_CASE("abode retell limit and is_mine")
{
	InMemoryTemporality temporality;
	const id::Soul me{10};
	const id::Soul other{20};

	seed_man(temporality, me, test_token("c0ffee00c0ffee00c0ffee00c0ffee00"), test_name("menameaa"));
	seed_man(temporality, other, test_token("deadbeefdeadbeefdeadbeefdeadbeef"), test_name("peername"));
	World world(temporality);

	temporality.fix(world.abode().id(), other, "peer");
	temporality.fix(world.abode().id(), me, "mine");
	temporality.fix(world.abode().id(), other, "peer2");

	const auto zero_limit = world.abode().retell(me, 0);
	REQUIRE(std::holds_alternative<DomainError>(zero_limit));
	CHECK(std::get<DomainError>(zero_limit).code == DomainErrorCode::InvalidArgument);

	const auto ok = world.abode().retell(me, 2);
	REQUIRE(std::holds_alternative<std::vector<RetoldLetter>>(ok));

	const auto& items = std::get<std::vector<RetoldLetter>>(ok);
	REQUIRE(items.size() == 2);
	CHECK(items[0].letter.body() == "mine");
	CHECK(items[0].author_name == "menameaa");
	CHECK(items[0].is_mine);
	CHECK(items[1].letter.body() == "peer2");
	CHECK(items[1].author_name == "peername");
	CHECK_FALSE(items[1].is_mine);
}


TEST_CASE("abode retell caps limit")
{
	InMemoryTemporality temporality;
	const id::Soul author{1};

	seed_man(temporality, author, test_token("feedfacefeedfacefeedfacefeedface"), test_name("authoraa"));
	World world(temporality);

	for (int i = 0; i < 5; ++i)
		temporality.fix(world.abode().id(), author, "m");

	const auto ok = world.abode().retell(author, Abode::MaxRetellLimit + 50);
	REQUIRE(std::holds_alternative<std::vector<RetoldLetter>>(ok));
	CHECK(std::get<std::vector<RetoldLetter>>(ok).size() == 5);
}
