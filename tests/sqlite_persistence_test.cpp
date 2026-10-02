#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "acts/creation.h"
#include "beings/deed.h"
#include "beings/immanents/obedience.h"
#include "beings/letter.h"
#include "beings/immanents/novice.h"
#include "beings/immanents/shepherding.h"
#include "beings/immanents/testator.h"
#include "beings/immanents/witness.h"
#include "sqlite_persistence_bundle.h"

#include "identity/abode.h"
#include "identity/place.h"
#include "values/abode_name.h"
#include "values/device_token.h"
#include "identity/soul.h"
#include "identity/tie.h"
#include "identity/word.h"
#include "values/soul_name.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>


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

		man_a.say("from-peer");
		man_b.say("from-me");

		const auto& witness_a = static_cast<const Witness&>(man_a);
		const auto letters = world.contemplation(witness_a.Soul::id()).letters();
		REQUIRE(letters.size() == 1);
		CHECK(letters[0].saying().body() == "from-peer");
		CHECK(letters[0].author().id() == man_a.Soul::id());
		CHECK(world.soul(letters[0].author().id()).name() == *name_a);

		// man_b spoke in his own abode; check via spatiality/eternity on place_a only has man_a
		const auto placed = bundle.spatiality().placements(witness_a.abode().id(), 10);
		REQUIRE(placed.size() == 1);

		CHECK(static_cast<const Witness&>(man_a).abode().id() !=
			  static_cast<const Witness&>(man_b).abode().id());

		// One abode per soul: what is kept decides.
		REQUIRE(bundle.spatiality().abode(man_a.Soul::id()));
		CHECK(id::Place{bundle.spatiality().abode(man_a.Soul::id())->id().value()} == *abode_a_place);
		CHECK_THROWS_AS(bundle.spatiality().abide(man_a.Soul::id(), AbodeName{"second"}),
						std::logic_error);
		CHECK_FALSE(bundle.spatiality().abode(id::Soul{999999}));

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

		const auto letters = world.contemplation(man_a_reloaded.Soul::id()).letters();
		REQUIRE(letters.size() == 1);
		CHECK(letters[0].saying().body() == "from-peer");

		CHECK(world.knows(*soul_b_id));
		CHECK(world.soul(*soul_b_id).name() == *name_b);

		CHECK_FALSE(world.knows(id::Soul{999999}));
		CHECK_FALSE(world.knows(id::Vessel{999999}));

		// A man is gathered from two dimensions: Eternity keeps the soul with its
		// name, Temporality the vessel and the embodiment that joins them.
		const auto souls = bundle.eternity().souls();
		const auto vessels = bundle.temporality().vessels();
		const auto embodiments = bundle.temporality().embodiments();
		REQUIRE(souls.size() == 3);
		REQUIRE(vessels.size() == 3);
		REQUIRE(embodiments.size() == 3);
		CHECK(souls.front().id() == *soul_a_id);
		CHECK(souls.front().name() == *name_a);
		CHECK(embodiments.front().soul() == *soul_a_id);
		CHECK(embodiments.front().vessel() == man_a_reloaded.Vessel::id());
		CHECK(vessels.front().id() == man_a_reloaded.Vessel::id());
	}

	::unlink((prefix + ".eternity.db").c_str());
	::unlink((prefix + ".space.db").c_str());
	::unlink((prefix + ".time.db").c_str());
}


TEST_CASE("sqlite keeps a deed and its execution across reopen")
{
	using namespace will;
	using namespace will::domain;

	const std::string prefix = "/tmp/will-sqlite-deed-test-" + std::to_string(getpid());
	::unlink((prefix + ".eternity.db").c_str());
	::unlink((prefix + ".space.db").c_str());
	::unlink((prefix + ".time.db").c_str());

	const DeviceToken token_novice = *DeviceToken::parse("aaaa1234aaaa1234aaaa1234aaaa1234");
	const DeviceToken token_testator = *DeviceToken::parse("bbbb1234bbbb1234bbbb1234bbbb1234");
	std::optional<id::Word> open_id;
	std::optional<id::Word> done_id;

	{
		SqlitePersistenceBundle bundle(prefix);
		World& world = bundle.world();

		const auto& novice = static_cast<const Novice&>(world.welcome(token_novice));
		const auto& testator = static_cast<const Testator&>(world.welcome(token_testator));

		novice.supplicate(testator);
		testator.accept(testator.supplication(novice));

		const Deed open = testator.will(testator.shepherding(novice), "fast");
		const Deed willed = testator.will(testator.shepherding(novice), "pray");
		CHECK(open.open());
		CHECK(&willed.tie().novice() == &novice);
		open_id = open.id();
		done_id = willed.id();

		novice.execute(willed);
		CHECK_THROWS_AS(novice.execute(willed), std::logic_error);
		CHECK(bundle.temporality().executions({*open_id, *done_id}).size() == 1);

		// Deeds are placed in the tie, not among the letters of an abode.
		CHECK(world.contemplation(novice.Soul::id()).letters().empty());
	}

	{
		SqlitePersistenceBundle bundle(prefix);
		World& world = bundle.world();

		const auto& novice = static_cast<const Novice&>(world.welcome(token_novice));
		const auto& testator = static_cast<const Testator&>(world.welcome(token_testator));

		// The reborn tie shows the same deeds to both sides, oldest first.
		const std::vector<Deed> shown = novice.obedience(testator).deeds(novice);
		REQUIRE(shown.size() == 2);
		const Deed& open = shown[0];
		const Deed& done = shown[1];

		CHECK(open.id() == *open_id);
		CHECK(open.open());
		CHECK(open.saying().body() == "fast");
		CHECK(&open.tie().testator() == &testator);

		CHECK(done.id() == *done_id);
		CHECK(done.executed());
		CHECK(done.saying().body() == "pray");
		CHECK(testator.shepherding(novice).deeds(testator).size() == 2);
		CHECK_THROWS_AS(novice.execute(done), std::logic_error);

		novice.execute(open);
		CHECK(novice.obedience(testator).deeds(novice).front().executed());
	}

	::unlink((prefix + ".eternity.db").c_str());
	::unlink((prefix + ".space.db").c_str());
	::unlink((prefix + ".time.db").c_str());
}


TEST_CASE("sqlite keeps supplications, rejections and ties across reopen")
{
	using namespace will;
	using namespace will::domain;

	const std::string prefix = "/tmp/will-sqlite-supplication-test-" + std::to_string(getpid());
	::unlink((prefix + ".eternity.db").c_str());
	::unlink((prefix + ".space.db").c_str());
	::unlink((prefix + ".time.db").c_str());

	const DeviceToken token_a = *DeviceToken::parse("aaaa1234aaaa1234aaaa1234aaaa1234");
	const DeviceToken token_b = *DeviceToken::parse("bbbb1234bbbb1234bbbb1234bbbb1234");
	const DeviceToken token_c = *DeviceToken::parse("cccc1234cccc1234cccc1234cccc1234");

	{
		SqlitePersistenceBundle bundle(prefix);
		World& world = bundle.world();

		const auto& a = static_cast<const Testator&>(world.welcome(token_a));
		const auto& b = static_cast<const Testator&>(world.welcome(token_b));
		const auto& c = static_cast<const Testator&>(world.welcome(token_c));

		// a asks b twice: rejected, then asked again and left awaiting.
		a.supplicate(b);
		CHECK_THROWS_AS(a.supplicate(b), std::logic_error);
		b.reject(b.supplication(a));
		CHECK(bundle.temporality().supplications(b.Soul::id()).empty());
		CHECK_THROWS_AS(bundle.temporality().reject(a.Soul::id(), b.Soul::id()), std::invalid_argument);
		a.supplicate(b);

		// c asks b and is accepted.
		c.supplicate(b);
		CHECK(bundle.temporality().supplications(b.Soul::id()).size() == 2);
		const Shepherding& shepherding = b.accept(b.supplication(c));
		CHECK(&shepherding.novice() == &c);
		CHECK(b.supplications().size() == 1);
		CHECK_THROWS_AS(c.supplicate(b), std::logic_error);
	}

	{
		SqlitePersistenceBundle bundle(prefix);
		World& world = bundle.world();

		const auto& a = static_cast<const Testator&>(world.welcome(token_a));
		const auto& b = static_cast<const Testator&>(world.welcome(token_b));
		const auto& c = static_cast<const Testator&>(world.welcome(token_c));

		// The awaiting supplication is reborn on the addressee's heap; the tie is living.
		REQUIRE(b.supplications().size() == 1);
		CHECK(&b.supplication(a).suppliant() == &a);
		CHECK(&b.shepherding(c).novice() == &c);
		CHECK_THROWS_AS(b.shepherding(a), std::invalid_argument);

		const Shepherding& shepherding = b.accept(b.supplication(a));
		CHECK(&shepherding.novice() == &a);
		CHECK(b.supplications().empty());
		CHECK(bundle.spatiality().ties().size() == 2);
	}

	::unlink((prefix + ".eternity.db").c_str());
	::unlink((prefix + ".space.db").c_str());
	::unlink((prefix + ".time.db").c_str());
}
