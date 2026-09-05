#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "domain_fakes.h"

#include "entities/world.h"
#include "identity/abode.h"
#include "values/device_token.h"
#include "values/timestamp.h"
#include "values/soul_name.h"
#include "errors/auth_error.h"
#include "errors/domain_error.h"
#include "usecases/authenticate_device.h"
#include "usecases/fetch_letter_history.h"
#include "usecases/send_letter.h"

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


TEST_CASE("authenticate_device creates soul")
{
	InMemoryEternity eternity;
	World world(eternity);
	AuthenticateDevice authenticate(world);

	const DeviceToken token = DeviceToken::generate();
	const auto result = authenticate.execute(AuthenticateDeviceInput{token.text()});
	REQUIRE(std::holds_alternative<AuthenticateDeviceSuccess>(result));

	const AuthenticateDeviceSuccess& success = std::get<AuthenticateDeviceSuccess>(result);
	CHECK(success.man.soul_id().value() > 0);
	CHECK(success.man.id().value() > 0);
	CHECK(success.man.vessel_id().value() > 0);

	const std::optional<Vessel> vessel = world.find_vessel_by_token(token);
	REQUIRE(vessel.has_value());
	CHECK(vessel->id() == success.man.vessel_id());
	CHECK(world.find_man_by_vessel(*vessel).id() == success.man.id());
	CHECK(world.find_man_by_vessel(*vessel).soul_id() == success.man.soul_id());

	const std::optional<Soul> soul = world.find_by_id(success.man.soul_id());
	REQUIRE(soul.has_value());
	CHECK(SoulName::parse(soul->name().text()));
}


TEST_CASE("authenticate_device existing soul")
{
	InMemoryEternity eternity;
	seed_man(eternity, id::Soul{42}, test_token("abcd1234abcd1234abcd1234abcd1234"), test_name("oldname1"));
	World world(eternity);

	AuthenticateDevice authenticate(world);
	const auto result = authenticate.execute(AuthenticateDeviceInput{"abcd1234abcd1234abcd1234abcd1234"});
	REQUIRE(std::holds_alternative<AuthenticateDeviceSuccess>(result));
	CHECK(std::get<AuthenticateDeviceSuccess>(result).man.soul_id() == id::Soul{42});
}


TEST_CASE("authenticate_device keeps existing name")
{
	InMemoryEternity eternity;
	seed_man(eternity, id::Soul{7}, test_token("abcd1234abcd1234abcd1234abcd1234"), test_name("keptname"));
	World world(eternity);

	AuthenticateDevice authenticate(world);
	const auto result = authenticate.execute(AuthenticateDeviceInput{"abcd1234abcd1234abcd1234abcd1234"});
	REQUIRE(std::holds_alternative<AuthenticateDeviceSuccess>(result));

	const std::optional<Soul> soul = world.find_by_id(id::Soul{7});
	REQUIRE(soul.has_value());
	CHECK(soul->name() == test_name("keptname"));
}


TEST_CASE("authenticate_device invalid token")
{
	InMemoryEternity eternity;
	World world(eternity);
	AuthenticateDevice authenticate(world);

	const auto result = authenticate.execute(AuthenticateDeviceInput{"short"});
	REQUIRE(std::holds_alternative<AuthError>(result));
	CHECK(std::get<AuthError>(result) == AuthError::InvalidToken);
}


TEST_CASE("send_letter persists and notifies")
{
	InMemoryLetterRepository letters;
	FakeParticipantNotifier notifier;

	const id::Soul author{7};

	SendLetter send(letters, notifier);
	const Letter saved = send.execute(SendLetterInput{author, id::Abode::global(), "hello", Timestamp{900}});

	CHECK(saved.id().value() > 0);
	CHECK(saved.author_id() == author);
	CHECK(saved.body() == "hello");
	CHECK(saved.created_at() == Timestamp{900});

	const auto loaded = letters.load_last(id::Abode::global(), 10);
	REQUIRE(loaded.size() == 1);
	CHECK(loaded[0].body() == "hello");

	REQUIRE(notifier.notifications_.size() == 1);
	CHECK(notifier.notifications_[0].id() == saved.id());
}


TEST_CASE("fetch_letter_history limit and is_mine")
{
	InMemoryLetterRepository letters;
	InMemoryEternity eternity;
	const id::Soul me{10};
	const id::Soul other{20};

	seed_man(eternity, me, test_token("c0ffee00c0ffee00c0ffee00c0ffee00"), test_name("menameaa"));
	seed_man(eternity, other, test_token("deadbeefdeadbeefdeadbeefdeadbeef"), test_name("peername"));
	World world(eternity);
	const id::Abode abode = world.abode().id;

	letters.append(abode, other, "peer", Timestamp{1});
	letters.append(abode, me, "mine", Timestamp{2});
	letters.append(abode, other, "peer2", Timestamp{3});

	FetchLetterHistory fetch(letters, world);

	const auto zero_limit = fetch.execute(FetchLetterHistoryInput{me, abode, 0});
	REQUIRE(std::holds_alternative<DomainError>(zero_limit));
	CHECK(std::get<DomainError>(zero_limit).code == DomainErrorCode::InvalidArgument);

	const auto ok = fetch.execute(FetchLetterHistoryInput{me, abode, 2});
	REQUIRE(std::holds_alternative<FetchLetterHistoryResult>(ok));

	const FetchLetterHistoryResult& result = std::get<FetchLetterHistoryResult>(ok);
	REQUIRE(result.items.size() == 2);
	CHECK(result.items[0].letter.body() == "mine");
	CHECK(result.items[0].author_name == "menameaa");
	CHECK(result.items[0].is_mine);
	CHECK(result.items[1].letter.body() == "peer2");
	CHECK(result.items[1].author_name == "peername");
	CHECK_FALSE(result.items[1].is_mine);
}


TEST_CASE("fetch_letter_history caps limit")
{
	InMemoryLetterRepository letters;
	InMemoryEternity eternity;
	const id::Soul author{1};

	seed_man(eternity, author, test_token("feedfacefeedfacefeedfacefeedface"), test_name("authoraa"));
	World world(eternity);
	const id::Abode abode = world.abode().id;

	for (int i = 0; i < 5; ++i)
		letters.append(abode, author, "m", Timestamp{i});

	FetchLetterHistory fetch(letters, world);
	const auto ok = fetch.execute(FetchLetterHistoryInput{
		author, abode, FetchLetterHistory::MaxHistoryRequestLimit + 50});
	REQUIRE(std::holds_alternative<FetchLetterHistoryResult>(ok));
	CHECK(std::get<FetchLetterHistoryResult>(ok).items.size() == 5);
}
