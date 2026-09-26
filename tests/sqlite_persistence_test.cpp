#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "acts/creation.h"
#include "beings/immanents/witness.h"
#include "sqlite_persistence_bundle.h"

#include "identity/abode.h"
#include "identity/place.h"
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

	const std::string prefix = "/tmp/will-sqlite-persistence-test-" + std::to_string(getpid());
	::unlink((prefix + ".eternity.db").c_str());
	::unlink((prefix + ".space.db").c_str());
	::unlink((prefix + ".time.db").c_str());

	std::optional<id::Soul> soul_a_id;
	std::optional<id::Soul> soul_b_id;
	std::optional<id::Soul> created_id;
	std::optional<id::Place> abode_a_place;
	std::optional<SoulName> name_a;
	std::optional<SoulName> name_b;
	std::optional<SoulName> name_created;
	const std::string token_text = "abcd1234abcd1234abcd1234abcd1234";

	{
		SqlitePersistenceBundle bundle(prefix);
		World& world = bundle.world();

		const DeviceToken token_a = *DeviceToken::parse("aaaa1234aaaa1234aaaa1234aaaa1234");
		const DeviceToken token_b = *DeviceToken::parse("bbbb1234bbbb1234bbbb1234bbbb1234");
		const DeviceToken token_created = *DeviceToken::parse(token_text);

		const Man& man_a = world.welcome(token_a);
		const Man& man_b = world.welcome(token_b);
		soul_a_id = man_a.Soul::id();
		soul_b_id = man_b.Soul::id();
		name_a = man_a.name();
		name_b = man_b.name();
		abode_a_place = static_cast<const Witness&>(man_a).abode().id();

		man_a.say(Word{"from-peer"});
		man_b.say(Word{"from-me"});

		const auto& witness_a = static_cast<const Witness&>(man_a);
		const auto letters = witness_a.retell(10);
		REQUIRE(letters.size() == 1);
		CHECK(letters[0].body() == "from-peer");
		CHECK(letters[0].author().id() == man_a.Soul::id());
		CHECK(world.soul(letters[0].author().id()).name() == *name_a);

		// man_b spoke in his own abode; check via spatiality/eternity on place_a only has man_a
		const auto placed = bundle.spatiality().placements(witness_a.abode().id(), 10);
		REQUIRE(placed.size() == 1);

		CHECK(static_cast<const Witness&>(man_a).abode().id() !=
			  static_cast<const Witness&>(man_b).abode().id());

		const Man& man_created = world.welcome(token_created);
		created_id = man_created.Soul::id();
		name_created = man_created.name();
		CHECK(man_created.Soul::id().value() > 0);
		CHECK(world.knows(man_created.Vessel::id()));
		CHECK(world.man(world.vessel(man_created.Vessel::id())).Soul::id() == man_created.Soul::id());
		CHECK(world.knows(man_created.Soul::id()));
	}

	{
		SqlitePersistenceBundle bundle(prefix);
		World& world = bundle.world();

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
		CHECK(static_cast<const Witness&>(man_a_reloaded).abode().dwells(man_a_reloaded));
		CHECK(static_cast<const Witness&>(man_a_reloaded).abode().id() == *abode_a_place);

		const auto letters = static_cast<const Witness&>(man_a_reloaded).retell(10);
		REQUIRE(letters.size() == 1);
		CHECK(letters[0].body() == "from-peer");

		CHECK(world.knows(*soul_b_id));
		CHECK(world.soul(*soul_b_id).name() == *name_b);

		CHECK_FALSE(world.knows(id::Soul{999999}));
		CHECK_FALSE(world.knows(id::Vessel{999999}));
	}

	::unlink((prefix + ".eternity.db").c_str());
	::unlink((prefix + ".space.db").c_str());
	::unlink((prefix + ".time.db").c_str());
}
