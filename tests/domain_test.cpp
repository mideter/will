#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "domain_fakes.h"

#include "entities/abode.h"
#include "entities/world.h"
#include "identity/abode.h"
#include "values/device_token.h"
#include "values/timestamp.h"
#include "values/soul_name.h"
#include "errors/domain_error.h"

#include <variant>


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
	InMemoryEternity eternity;
	World world(eternity);

	const DeviceToken token = DeviceToken::generate();
	const Man man = world.welcome(token);

	CHECK(man.soul_id().value() > 0);
	CHECK(man.id().value() > 0);
	CHECK(man.vessel_id().value() > 0);

	const std::optional<Vessel> vessel = world.find_vessel_by_token(token);
	REQUIRE(vessel.has_value());
	CHECK(vessel->id() == man.vessel_id());
	CHECK(world.find_man_by_vessel(*vessel).id() == man.id());
	CHECK(world.find_man_by_vessel(*vessel).soul_id() == man.soul_id());

	const std::optional<Soul> soul = world.find_by_id(man.soul_id());
	REQUIRE(soul.has_value());
	CHECK(SoulName::parse(soul->name().text()));
}


TEST_CASE("welcome existing man")
{
	InMemoryEternity eternity;
	seed_man(eternity, id::Soul{42}, test_token("abcd1234abcd1234abcd1234abcd1234"), test_name("oldname1"));
	World world(eternity);

	const Man man = world.welcome(test_token("abcd1234abcd1234abcd1234abcd1234"));
	CHECK(man.soul_id() == id::Soul{42});
}


TEST_CASE("welcome keeps existing name")
{
	InMemoryEternity eternity;
	seed_man(eternity, id::Soul{7}, test_token("abcd1234abcd1234abcd1234abcd1234"), test_name("keptname"));
	World world(eternity);

	(void)world.welcome(test_token("abcd1234abcd1234abcd1234abcd1234"));

	const std::optional<Soul> soul = world.find_by_id(id::Soul{7});
	REQUIRE(soul.has_value());
	CHECK(soul->name() == test_name("keptname"));
}


TEST_CASE("abode inscribe persists and notifies")
{
	InMemoryLetterRepository letters;
	FakeParticipantNotifier notifier;
	InMemoryEternity eternity;
	World world(eternity);
	Abode abode(id::Abode::global(), letters, notifier, world);

	const id::Soul author{7};
	const Letter saved = abode.inscribe(author, "hello", Timestamp{900});

	CHECK(saved.id().value() > 0);
	CHECK(saved.author_id() == author);
	CHECK(saved.body() == "hello");
	CHECK(saved.created_at() == Timestamp{900});

	const auto loaded = letters.load_last(abode.id(), 10);
	REQUIRE(loaded.size() == 1);
	CHECK(loaded[0].body() == "hello");

	REQUIRE(notifier.notifications_.size() == 1);
	CHECK(notifier.notifications_[0].id() == saved.id());
}


TEST_CASE("abode retell limit and is_mine")
{
	InMemoryLetterRepository letters;
	FakeParticipantNotifier notifier;
	InMemoryEternity eternity;
	const id::Soul me{10};
	const id::Soul other{20};

	seed_man(eternity, me, test_token("c0ffee00c0ffee00c0ffee00c0ffee00"), test_name("menameaa"));
	seed_man(eternity, other, test_token("deadbeefdeadbeefdeadbeefdeadbeef"), test_name("peername"));
	World world(eternity);
	Abode abode(world.abode_id(), letters, notifier, world);

	letters.append(abode.id(), other, "peer", Timestamp{1});
	letters.append(abode.id(), me, "mine", Timestamp{2});
	letters.append(abode.id(), other, "peer2", Timestamp{3});

	const auto zero_limit = abode.retell(me, 0);
	REQUIRE(std::holds_alternative<DomainError>(zero_limit));
	CHECK(std::get<DomainError>(zero_limit).code == DomainErrorCode::InvalidArgument);

	const auto ok = abode.retell(me, 2);
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
	InMemoryLetterRepository letters;
	FakeParticipantNotifier notifier;
	InMemoryEternity eternity;
	const id::Soul author{1};

	seed_man(eternity, author, test_token("feedfacefeedfacefeedfacefeedface"), test_name("authoraa"));
	World world(eternity);
	Abode abode(world.abode_id(), letters, notifier, world);

	for (int i = 0; i < 5; ++i)
		letters.append(abode.id(), author, "m", Timestamp{i});

	const auto ok = abode.retell(author, Abode::MaxRetellLimit + 50);
	REQUIRE(std::holds_alternative<std::vector<RetoldLetter>>(ok));
	CHECK(std::get<std::vector<RetoldLetter>>(ok).size() == 5);
}
