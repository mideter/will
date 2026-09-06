#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "entities/world.h"
#include "sqlite_database.h"
#include "sqlite_store.h"
#include "sqlite_letter_repository_impl.h"

#include "identity/abode.h"
#include "values/device_token.h"
#include "values/timestamp.h"
#include "identity/soul.h"
#include "values/soul_name.h"

#include <optional>
#include <string>
#include <unistd.h>


TEST_CASE("sqlite persistence survives reopen")
{
	using namespace will;
	using namespace will::domain;

	const std::string db_path = "/tmp/will-sqlite-persistence-test-" + std::to_string(getpid()) + ".db";
	::unlink(db_path.c_str());

	std::optional<id::Soul> soul_a_id;
	std::optional<id::Soul> soul_b_id;
	std::optional<id::Soul> created_id;
	std::optional<SoulName> name_a;
	std::optional<SoulName> name_b;
	std::optional<SoulName> name_created;
	const std::string token_text = "abcd1234abcd1234abcd1234abcd1234";

	{
		SqliteDatabase database(db_path);
		SqliteStore store(database);
		SqliteLetterRepositoryImpl letters(database);
		World world(store);

		const DeviceToken token_a = *DeviceToken::parse("aaaa1234aaaa1234aaaa1234aaaa1234");
		const DeviceToken token_b = *DeviceToken::parse("bbbb1234bbbb1234bbbb1234bbbb1234");
		const DeviceToken token_created = *DeviceToken::parse(token_text);

		const Man man_a = world.welcome(token_a);
		const Man man_b = world.welcome(token_b);
		const Soul soul_a = *world.find_by_id(man_a.soul_id());
		const Soul soul_b = *world.find_by_id(man_b.soul_id());
		soul_a_id = soul_a.id();
		soul_b_id = soul_b.id();
		name_a = soul_a.name();
		name_b = soul_b.name();

		const id::Abode abode = world.abode_id();
		letters.append(abode, soul_a.id(), "from-peer", Timestamp{1000});
		letters.append(abode, soul_b.id(), "from-me", Timestamp{2000});

		const auto rows = letters.load_last(abode, 10);
		REQUIRE(rows.size() == 2);
		CHECK(rows[0].body() == "from-peer");
		CHECK(rows[0].author_id() == soul_a.id());
		CHECK(world.find_by_id(rows[0].author_id())->name() == *name_a);
		CHECK(rows[1].body() == "from-me");
		CHECK(rows[1].author_id() == soul_b.id());
		CHECK(world.find_by_id(rows[1].author_id())->name() == *name_b);

		const Man man_created = world.welcome(token_created);
		const Soul created = *world.find_by_id(man_created.soul_id());
		created_id = created.id();
		name_created = created.name();
		CHECK(created.id().value() > 0);
		const std::optional<Vessel> vessel_created = world.find_vessel_by_token(token_created);
		REQUIRE(vessel_created.has_value());
		CHECK(world.find_man_by_vessel(*vessel_created).soul_id() == created.id());
		CHECK(world.find_by_id(created.id()).has_value());
	}

	{
		SqliteDatabase database(db_path);
		SqliteStore store(database);
		World world(store);

		const DeviceToken token_created = *DeviceToken::parse(token_text);
		const std::optional<Vessel> vessel_created = world.find_vessel_by_token(token_created);
		REQUIRE(vessel_created.has_value());
		CHECK(world.find_man_by_vessel(*vessel_created).soul_id() == *created_id);
		CHECK(world.find_by_id(*created_id)->name() == *name_created);

		const std::optional<Soul> a = world.find_by_id(*soul_a_id);
		REQUIRE(a.has_value());
		CHECK(a->name() == *name_a);
		const std::optional<Vessel> vessel_a =
			world.find_vessel_by_token(*DeviceToken::parse("aaaa1234aaaa1234aaaa1234aaaa1234"));
		REQUIRE(vessel_a.has_value());
		CHECK(world.find_man_by_vessel(*vessel_a).soul_id() == *soul_a_id);

		const std::optional<Soul> b = world.find_by_id(*soul_b_id);
		REQUIRE(b.has_value());
		CHECK(b->name() == *name_b);

		CHECK_FALSE(world.find_by_id(id::Soul{999999}).has_value());
		CHECK_FALSE(world.find_vessel_by_token(*DeviceToken::parse("ffffffffffffffffffffffffffffffff"))
						.has_value());
	}

	::unlink(db_path.c_str());
}
