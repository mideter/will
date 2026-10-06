#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "horizons/creation.h"
#include "words/behest.h"
#include "words/deed.h"
#include "places/obedience.h"
#include "words/letter.h"
#include "men/novice.h"
#include "places/shepherding.h"
#include "men/testator.h"
#include "men/witness.h"
#include "relations/contemplation.h"
#include "relations/friend.h"
#include "places/room.h"
#include "sqlite_persistence_bundle.h"

#include "identity/place.h"
#include "values/abode_name.h"
#include "values/device_token.h"
#include "identity/soul.h"
#include "identity/word.h"
#include "values/soul_name.h"

#include <sqlite3.h>

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>


namespace {


/// The letters a gaze beholds (the words of an abode are letters).
std::vector<std::shared_ptr<const will::domain::Letter>>
letters_seen_by(const std::shared_ptr<const will::domain::Contemplation>& gaze)
{
	std::vector<std::shared_ptr<const will::domain::Letter>> letters;
	for (const std::shared_ptr<const will::domain::Word>& word : gaze->words()) {
		if (auto letter = std::dynamic_pointer_cast<const will::domain::Letter>(word))
			letters.push_back(std::move(letter));
	}
	return letters;
}


} // namespace


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

		static_cast<const Witness&>(man_a).wake();
		static_cast<const Witness&>(man_b).wake();
		static_cast<const Witness&>(man_a).contemplate(*man_a.abode().room(man_a.abode()));
		static_cast<const Witness&>(man_b).contemplate(*man_b.abode().room(man_b.abode()));
		man_a.say("from-peer");
		man_b.say("from-me");

		const auto& witness_a = static_cast<const Witness&>(man_a);
		const auto letters = letters_seen_by(world.contemplation(witness_a.Soul::id()));
		REQUIRE(letters.size() == 1);
		CHECK(letters[0]->saying().body() == "from-peer");
		CHECK(letters[0]->author().id() == man_a.Soul::id());
		CHECK(world.soul(letters[0]->author().id()).name() == *name_a);

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

		// After the World awakens anew everyone is asleep until he comes.
		CHECK_FALSE(world.contemplation(man_a_reloaded.Soul::id()));
		static_cast<const Witness&>(man_a_reloaded).wake();
		static_cast<const Witness&>(man_a_reloaded).contemplate(*man_a_reloaded.abode().room(man_a_reloaded.abode()));

		const auto letters = letters_seen_by(world.contemplation(man_a_reloaded.Soul::id()));
		REQUIRE(letters.size() == 1);
		CHECK(letters[0]->saying().body() == "from-peer");

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


TEST_CASE("sqlite keeps a behest and its execution across reopen")
{
	using namespace will;
	using namespace will::domain;

	const std::string prefix = "/tmp/will-sqlite-behest-test-" + std::to_string(getpid());
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

		novice.admit(testator);
		testator.admit(novice);
		novice.supplicate(testator);
		testator.accept(*testator.supplication(novice));
		testator.wake();
		testator.contemplate(testator.shepherding(novice));
		novice.wake();
		novice.contemplate(novice.obedience(testator));

		const std::shared_ptr<const Behest> open_held = testator.will(testator.shepherding(novice), "fast");
		const Behest& open = *open_held;
		const std::shared_ptr<const Behest> willed_held = testator.will(testator.shepherding(novice), "pray");
		const Behest& willed = *willed_held;
		CHECK(&willed.tie().novice() == &novice);
		open_id = open.id();
		done_id = willed.id();

		novice.execute(willed);
		CHECK_THROWS_AS(novice.execute(willed), std::logic_error);
		CHECK(bundle.temporality().executions({*open_id, *done_id}).size() == 1);

		// Behests are placed in the tie, not among the letters of an abode.
		novice.wake();
		CHECK(letters_seen_by(world.contemplation(novice.Soul::id())).empty());
	}

	{
		SqlitePersistenceBundle bundle(prefix);
		World& world = bundle.world();

		const auto& novice = static_cast<const Novice&>(world.welcome(token_novice));
		const auto& testator = static_cast<const Testator&>(world.welcome(token_testator));

		// The reborn tie shows the same behests to both sides, oldest first.
		const std::vector<std::shared_ptr<const Behest>> shown = novice.obedience(testator).behests(novice);
		REQUIRE(shown.size() == 2);
		const Behest& open = *shown[0];
		const Behest& done = *shown[1];

		CHECK(open.id() == *open_id);
		CHECK(open.saying().body() == "fast");
		CHECK(&open.tie().testator() == &testator);

		CHECK(done.id() == *done_id);
		CHECK(done.saying().body() == "pray");
		CHECK(testator.shepherding(novice).behests(testator).size() == 2);

		// The deed fulfilling «pray» was kept, and is born again with it.
		const auto deeds = novice.obedience(testator).deeds(novice);
		REQUIRE(deeds.size() == 1);
		CHECK(deeds.front()->behest() == *done_id);
		CHECK(deeds.front()->saying().body() == "совершено");
		novice.wake();
		novice.contemplate(novice.obedience(testator));
		CHECK_THROWS_AS(novice.execute(done), std::logic_error);

		novice.execute(open);
		CHECK(novice.obedience(testator).deeds(novice).size() == 2);
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
		a.admit(b);
		b.admit(a);
		a.supplicate(b);
		CHECK_THROWS_AS(a.supplicate(b), std::logic_error);
		b.reject(*b.supplication(a));
		CHECK(bundle.temporality().supplications(b.Soul::id()).empty());
		CHECK_THROWS_AS(bundle.temporality().answer(a.Soul::id(), b.Soul::id(), matter::Answer::Form::Rejected),
						std::invalid_argument);
		a.supplicate(b);

		// c asks b and is accepted.
		c.admit(b);
		b.admit(c);
		c.supplicate(b);
		CHECK(bundle.temporality().supplications(b.Soul::id()).size() == 2);
		const Shepherding& shepherding = b.accept(*b.supplication(c));
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

		// The awaiting supplication is reborn on the addressee's heap; the accepted
		// one is answered and stays behind; the tie is living.
		CHECK(bundle.temporality().supplications(b.Soul::id()).size() == 1);
		REQUIRE(b.supplications().size() == 1);
		CHECK(&b.supplication(a)->suppliant() == &a);
		CHECK(&b.shepherding(c).novice() == &c);
		CHECK_THROWS_AS(b.shepherding(a), std::invalid_argument);

		const Shepherding& shepherding = b.accept(*b.supplication(a));
		CHECK(&shepherding.novice() == &a);
		CHECK(b.supplications().empty());
		CHECK(bundle.spatiality().ties().size() == 2);
	}

	::unlink((prefix + ".eternity.db").c_str());
	::unlink((prefix + ".space.db").c_str());
	::unlink((prefix + ".time.db").c_str());
}


TEST_CASE("sqlite answers the rejections kept before answers were")
{
	using namespace will;
	using namespace will::domain;

	const std::string prefix = "/tmp/will-sqlite-rejections-test-" + std::to_string(getpid());
	::unlink((prefix + ".eternity.db").c_str());
	::unlink((prefix + ".space.db").c_str());
	::unlink((prefix + ".time.db").c_str());

	// A time database as it was kept before: a rejected supplication of the pair 1 → 2.
	{
		sqlite3* db = nullptr;
		REQUIRE(sqlite3_open((prefix + ".time.db").c_str(), &db) == SQLITE_OK);
		REQUIRE(sqlite3_exec(db,
							 "CREATE TABLE supplications (id INTEGER PRIMARY KEY AUTOINCREMENT, "
							 "suppliant_soul_id INTEGER NOT NULL, addressee_soul_id INTEGER NOT NULL, "
							 "asked_at_ns INTEGER NOT NULL);"
							 "CREATE TABLE rejections (supplication_id INTEGER PRIMARY KEY, rejected_at_ns INTEGER NOT NULL);"
							 "INSERT INTO supplications (suppliant_soul_id, addressee_soul_id, asked_at_ns) VALUES (1, 2, 5);"
							 "INSERT INTO rejections (supplication_id, rejected_at_ns) VALUES (1, 6);",
							 nullptr, nullptr, nullptr)
				== SQLITE_OK);
		sqlite3_close(db);
	}

	{
		SqlitePersistenceBundle bundle(prefix);

		// The rejection became an answer: nothing awaits, and the pair may ask again.
		CHECK(bundle.temporality().supplications(id::Soul{2}).empty());
		bundle.temporality().ask(id::Soul{1}, id::Soul{2});
		CHECK(bundle.temporality().supplications(id::Soul{2}).size() == 1);
	}

	::unlink((prefix + ".eternity.db").c_str());
	::unlink((prefix + ".space.db").c_str());
	::unlink((prefix + ".time.db").c_str());
}


TEST_CASE("sqlite keeps the dwellers of an abode and their kind across reopen")
{
	using namespace will;
	using namespace will::domain;

	const std::string prefix = "/tmp/will-sqlite-dwellers-test-" + std::to_string(getpid());
	::unlink((prefix + ".eternity.db").c_str());
	::unlink((prefix + ".space.db").c_str());
	::unlink((prefix + ".time.db").c_str());

	const DeviceToken token_host = *DeviceToken::parse("aaaa1234aaaa1234aaaa1234aaaa1234");
	const DeviceToken token_man = *DeviceToken::parse("bbbb1234bbbb1234bbbb1234bbbb1234");

	{
		SqlitePersistenceBundle bundle(prefix);
		World& world = bundle.world();
		const Man& host = world.welcome(token_host);
		const Man& man = world.welcome(token_man);
		host.admit(man);
		host.regard(man, matter::Dweller::Kind::Friend);
	}

	{
		SqlitePersistenceBundle bundle(prefix);
		World& world = bundle.world();
		const Man& host = world.welcome(token_host);
		const Man& man = world.welcome(token_man);
		CHECK(std::dynamic_pointer_cast<const Friend>(host.abode().dweller(man)));
		CHECK_FALSE(man.abode().dwells(host));
		REQUIRE(bundle.spatiality().dwellers().size() == 1);

		// Each abode keeps its one cell across reopen.
		REQUIRE(host.abode().rooms().size() == 1);
		CHECK(host.abode().rooms().front().get().name() == "Келья");
		CHECK(bundle.spatiality().rooms(host.abode().id()).size() == 1);
	}

	::unlink((prefix + ".eternity.db").c_str());
	::unlink((prefix + ".space.db").c_str());
	::unlink((prefix + ".time.db").c_str());
}
