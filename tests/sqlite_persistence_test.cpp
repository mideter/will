#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "entities/world.h"
#include "sqlite_database.h"
#include "sqlite_temporality.h"

#include "identity/abode.h"
#include "values/device_token.h"
#include "identity/soul.h"
#include "values/soul_name.h"
#include "values/word.h"

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
		SqliteTemporality temporality(database);
		World world(temporality);

		const DeviceToken token_a = *DeviceToken::parse("aaaa1234aaaa1234aaaa1234aaaa1234");
		const DeviceToken token_b = *DeviceToken::parse("bbbb1234bbbb1234bbbb1234bbbb1234");
		const DeviceToken token_created = *DeviceToken::parse(token_text);

		const Man& man_a = world.welcome(token_a);
		const Man& man_b = world.welcome(token_b);
		soul_a_id = man_a.Soul::id();
		soul_b_id = man_b.Soul::id();
		name_a = man_a.name();
		name_b = man_b.name();

		const id::Abode abode = world.abode().id();
		temporality.fix(abode, man_a.Soul::id(), Word{"from-peer"});
		temporality.fix(abode, man_b.Soul::id(), Word{"from-me"});

		const auto rows = temporality.letters(abode, 10);
		REQUIRE(rows.size() == 2);
		CHECK(rows[0].body() == "from-peer");
		CHECK(rows[0].author_id() == man_a.Soul::id());
		CHECK(world.soul(rows[0].author_id()).name() == *name_a);
		CHECK(rows[1].body() == "from-me");
		CHECK(rows[1].author_id() == man_b.Soul::id());
		CHECK(world.soul(rows[1].author_id()).name() == *name_b);

		const Man& man_created = world.welcome(token_created);
		created_id = man_created.Soul::id();
		name_created = man_created.name();
		CHECK(man_created.Soul::id().value() > 0);
		CHECK(world.knows(man_created.Vessel::id()));
		CHECK(world.man(world.vessel(man_created.Vessel::id())).Soul::id() == man_created.Soul::id());
		CHECK(world.knows(man_created.Soul::id()));
	}

	{
		SqliteDatabase database(db_path);
		SqliteTemporality temporality(database);
		World world(temporality);

		const DeviceToken token_created = *DeviceToken::parse(token_text);
		const Man& reloaded = world.welcome(token_created);
		CHECK(reloaded.Soul::id() == *created_id);
		CHECK(world.knows(reloaded.Vessel::id()));
		CHECK(world.man(world.vessel(reloaded.Vessel::id())).Soul::id() == *created_id);
		CHECK(world.soul(*created_id).name() == *name_created);

		CHECK(world.knows(*soul_a_id));
		CHECK(world.soul(*soul_a_id).name() == *name_a);
		const DeviceToken token_a = *DeviceToken::parse("aaaa1234aaaa1234aaaa1234aaaa1234");
		const Man& man_a_reloaded = world.welcome(token_a);
		CHECK(man_a_reloaded.Soul::id() == *soul_a_id);
		CHECK(world.man(world.vessel(man_a_reloaded.Vessel::id())).Soul::id() == *soul_a_id);

		CHECK(world.knows(*soul_b_id));
		CHECK(world.soul(*soul_b_id).name() == *name_b);

		CHECK_FALSE(world.knows(id::Soul{999999}));
		CHECK_FALSE(world.knows(id::Vessel{999999}));
	}

	::unlink(db_path.c_str());
}
