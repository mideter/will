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
#include "relations/neighbour.h"
#include "places/room.h"
#include "sqlite_persistence_bundle.h"
#include "domain_fakes.h"

#include "identity/place.h"
#include "values/abode_name.h"
#include "values/device_token.h"
#include "identity/soul.h"
#include "identity/word.h"
#include "values/soul_name.h"

#include <sqlite3.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unistd.h>
#include <utility>
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


/// An empty directory for the databases of one test, gone when the test ends —
/// even when it fails.
class ScratchDirectory {
public:
	explicit ScratchDirectory(std::string path)
		: path_(std::move(path))
	{
		std::filesystem::remove_all(path_);
		std::filesystem::create_directories(path_);
	}

	~ScratchDirectory()
	{
		std::error_code error;
		std::filesystem::remove_all(path_, error);
	}

	ScratchDirectory(const ScratchDirectory&) = delete;
	ScratchDirectory& operator=(const ScratchDirectory&) = delete;

private:
	std::string path_;
};


using will::domain::test::admit_at_gates;
using will::domain::test::regard_in_upper_room;
using will::domain::test::born;

} // namespace


TEST_CASE("sqlite persistence survives reopen")
{
	using namespace will;
	using namespace will::domain;

	const std::string directory = "/tmp/will-sqlite-persistence-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

	std::optional<id::Soul> soul_a_id;
	std::optional<id::Soul> soul_b_id;
	std::optional<id::Soul> created_id;
	std::optional<id::Place> abode_a_place;
	std::optional<SoulName> name_a;
	std::optional<SoulName> name_b;
	std::optional<SoulName> name_created;
	const std::string token_text = "abcd1234abcd1234abcd1234abcd1234";

	{
		SqlitePersistenceBundle bundle(directory);
		World& world = bundle.world();

		const DeviceToken token_a = *DeviceToken::parse("aaaa1234aaaa1234aaaa1234aaaa1234");
		const DeviceToken token_b = *DeviceToken::parse("bbbb1234bbbb1234bbbb1234bbbb1234");
		const DeviceToken token_created = *DeviceToken::parse(token_text);

		const Man& man_a = born(world, token_a);
		const Man& man_b = born(world, token_b);
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

		const Man& man_created = born(world, token_created);
		created_id = man_created.Soul::id();
		name_created = man_created.name();
		CHECK(man_created.Soul::id().value() > 0);
		CHECK(world.knows(man_created.Vessel::id()));
		CHECK(world.man(world.vessel(man_created.Vessel::id())).Soul::id() == man_created.Soul::id());
		CHECK(world.knows(man_created.Soul::id()));
	}

	{
		SqlitePersistenceBundle bundle(directory);
		World& world = bundle.world();

		const DeviceToken token_created = *DeviceToken::parse(token_text);
		const Man& reloaded = born(world, token_created);
		CHECK(reloaded.Soul::id() == *created_id);
		CHECK(world.knows(reloaded.Vessel::id()));
		CHECK(world.man(world.vessel(reloaded.Vessel::id())).Soul::id() == *created_id);
		CHECK(world.soul(*created_id).name() == *name_created);

		CHECK(world.knows(*soul_a_id));
		CHECK(world.soul(*soul_a_id).name() == *name_a);
		const DeviceToken token_a = *DeviceToken::parse("aaaa1234aaaa1234aaaa1234aaaa1234");
		const Man& man_a_reloaded = born(world, token_a);
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
		// The forefather is the first of them.
		REQUIRE(souls.size() == 4);
		REQUIRE(vessels.size() == 4);
		REQUIRE(embodiments.size() == 4);
		CHECK(souls[1].id() == *soul_a_id);
		CHECK(souls[1].name() == *name_a);
		CHECK(embodiments[1].soul() == *soul_a_id);
		CHECK(embodiments[1].vessel() == man_a_reloaded.Vessel::id());
		CHECK(vessels[1].id() == man_a_reloaded.Vessel::id());
	}

}


TEST_CASE("sqlite keeps a behest and its execution across reopen")
{
	using namespace will;
	using namespace will::domain;

	const std::string directory = "/tmp/will-sqlite-behest-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

	const DeviceToken token_novice = *DeviceToken::parse("aaaa1234aaaa1234aaaa1234aaaa1234");
	const DeviceToken token_testator = *DeviceToken::parse("bbbb1234bbbb1234bbbb1234bbbb1234");
	std::optional<id::Word> open_id;
	std::optional<id::Word> done_id;

	{
		SqlitePersistenceBundle bundle(directory);
		World& world = bundle.world();

		const auto& novice = static_cast<const Novice&>(born(world, token_novice));
		const auto& testator = static_cast<const Testator&>(born(world, token_testator));

		admit_at_gates(world, novice, testator);
		admit_at_gates(world, testator, novice);
		novice.supplicate(testator);
		testator.accept(*testator.supplication(novice));
		testator.wake();
		testator.contemplate(*testator.abode().room(testator.shepherding(novice)));
		novice.wake();
		novice.contemplate(*novice.abode().room(novice.obedience(testator)));

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
		SqlitePersistenceBundle bundle(directory);
		World& world = bundle.world();

		const auto& novice = static_cast<const Novice&>(born(world, token_novice));
		const auto& testator = static_cast<const Testator&>(born(world, token_testator));

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
		novice.contemplate(*novice.abode().room(novice.obedience(testator)));
		CHECK_THROWS_AS(novice.execute(done), std::logic_error);

		novice.execute(open);
		CHECK(novice.obedience(testator).deeds(novice).size() == 2);
	}

}


TEST_CASE("sqlite keeps supplications, rejections and ties across reopen")
{
	using namespace will;
	using namespace will::domain;

	const std::string directory = "/tmp/will-sqlite-supplication-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

	const DeviceToken token_a = *DeviceToken::parse("aaaa1234aaaa1234aaaa1234aaaa1234");
	const DeviceToken token_b = *DeviceToken::parse("bbbb1234bbbb1234bbbb1234bbbb1234");
	const DeviceToken token_c = *DeviceToken::parse("cccc1234cccc1234cccc1234cccc1234");

	{
		SqlitePersistenceBundle bundle(directory);
		World& world = bundle.world();

		const auto& a = static_cast<const Testator&>(born(world, token_a));
		const auto& b = static_cast<const Testator&>(born(world, token_b));
		const auto& c = static_cast<const Testator&>(born(world, token_c));

		// a asks b twice: rejected, then asked again and left awaiting.
		admit_at_gates(world, a, b);
		admit_at_gates(world, b, a);
		a.supplicate(b);
		CHECK_THROWS_AS(a.supplicate(b), std::logic_error);
		b.reject(*b.supplication(a));
		CHECK(bundle.temporality().supplications(b.Soul::id()).empty());
		CHECK_THROWS_AS(bundle.temporality().answer(a.Soul::id(), b.Soul::id(), matter::Answer::Form::Rejected),
						std::invalid_argument);
		a.supplicate(b);

		// c asks b and is accepted.
		admit_at_gates(world, c, b);
		admit_at_gates(world, b, c);
		c.supplicate(b);
		CHECK(bundle.temporality().supplications(b.Soul::id()).size() == 2);
		const Shepherding& shepherding = b.accept(*b.supplication(c));
		CHECK(&shepherding.novice() == &c);
		CHECK(b.supplications().size() == 1);
		CHECK_THROWS_AS(c.supplicate(b), std::logic_error);
	}

	{
		SqlitePersistenceBundle bundle(directory);
		World& world = bundle.world();

		const auto& a = static_cast<const Testator&>(born(world, token_a));
		const auto& b = static_cast<const Testator&>(born(world, token_b));
		const auto& c = static_cast<const Testator&>(born(world, token_c));

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

}


TEST_CASE("sqlite answers the rejections kept before answers were")
{
	using namespace will;
	using namespace will::domain;

	const std::string directory = "/tmp/will-sqlite-rejections-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

	// A time database as it was kept before: a rejected supplication of the pair 1 → 2.
	{
		sqlite3* db = nullptr;
		REQUIRE(sqlite3_open((directory + "/will.time.db").c_str(), &db) == SQLITE_OK);
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
		SqlitePersistenceBundle bundle(directory);

		// The rejection became an answer: nothing awaits, and the pair may ask again.
		CHECK(bundle.temporality().supplications(id::Soul{2}).empty());
		bundle.temporality().ask(id::Soul{1}, id::Soul{2});
		CHECK(bundle.temporality().supplications(id::Soul{2}).size() == 1);
	}

}


TEST_CASE("sqlite keeps the dwellers of an abode and their kind across reopen")
{
	using namespace will;
	using namespace will::domain;

	const std::string directory = "/tmp/will-sqlite-dwellers-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

	const DeviceToken token_host = *DeviceToken::parse("aaaa1234aaaa1234aaaa1234aaaa1234");
	const DeviceToken token_man = *DeviceToken::parse("bbbb1234bbbb1234bbbb1234bbbb1234");

	{
		SqlitePersistenceBundle bundle(directory);
		World& world = bundle.world();
		const Man& host = born(world, token_host);
		const Man& man = born(world, token_man);
		admit_at_gates(world, host, man);
		regard_in_upper_room(world, host, man, matter::Dweller::Kind::Friend);
	}

	{
		SqlitePersistenceBundle bundle(directory);
		World& world = bundle.world();
		const Man& host = born(world, token_host);
		const Man& man = born(world, token_man);
		CHECK(std::dynamic_pointer_cast<const Friend>(host.abode().dweller(man)));
		CHECK_FALSE(man.abode().dwells(host));
		// In the host's abode dwell the man and the host's father by flesh.
		CHECK(host.abode().dwellers().size() == 2);
		CHECK(std::dynamic_pointer_cast<const Neighbour>(host.abode().dweller(*host.father(matter::Fatherhood::Line::Flesh))));

		// Each abode keeps its standard rooms across reopen: the cell, the gates,
		// the upper room.
		REQUIRE(host.abode().rooms().size() == 3);
		CHECK(host.abode().rooms()[0].get().name() == "Слово");
		CHECK(host.abode().rooms()[1].get().name() == "Врата");
		CHECK(host.abode().rooms()[2].get().name() == "Горница");
		CHECK(bundle.spatiality().rooms(host.abode().id()).size() == 3);
	}

}


TEST_CASE("sqlite gives the rooms kept before aspects were the aspect of words")
{
	using namespace will;

	const std::string directory = "/tmp/will-sqlite-room-aspect-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

	// A space database as it was kept before: a room without an aspect.
	{
		sqlite3* db = nullptr;
		REQUIRE(sqlite3_open((directory + "/will.space.db").c_str(), &db) == SQLITE_OK);
		REQUIRE(sqlite3_exec(db,
							 "CREATE TABLE rooms (id INTEGER PRIMARY KEY, abode_id INTEGER NOT NULL, "
							 "place_id INTEGER NOT NULL, part INTEGER NOT NULL, UNIQUE (abode_id, place_id));"
							 "INSERT INTO rooms (id, abode_id, place_id, part) VALUES (7, 3, 3, 1);",
							 nullptr, nullptr, nullptr)
				== SQLITE_OK);
		sqlite3_close(db);
	}

	{
		SqlitePersistenceBundle bundle(directory);
		const auto rooms = bundle.spatiality().rooms(domain::id::Place{3});
		REQUIRE(rooms.size() == 1);
		CHECK(rooms.front().id() == domain::id::Place{7});
		CHECK(rooms.front().aspect() == domain::matter::Room::Aspect::Words);
		CHECK(rooms.front().part() == domain::matter::Room::Part::Outer);

		// Another aspect of the same place may now be kept beside it.
		bundle.spatiality().furnish(domain::id::Place{3}, domain::id::Place{3}, domain::matter::Room::Aspect::Threshold);
		CHECK(bundle.spatiality().rooms(domain::id::Place{3}).size() == 2);
	}

}


TEST_CASE("sqlite lets the birth rooms kept before go: one is born through the gates")
{
	using namespace will;

	const std::string directory = "/tmp/will-sqlite-birth-room-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

	// A space database as it was kept before: the cell, the gates, the upper room, the birth room.
	{
		sqlite3* db = nullptr;
		REQUIRE(sqlite3_open((directory + "/will.space.db").c_str(), &db) == SQLITE_OK);
		REQUIRE(sqlite3_exec(db,
							 "CREATE TABLE rooms (id INTEGER PRIMARY KEY, abode_id INTEGER NOT NULL, "
							 "place_id INTEGER NOT NULL, aspect INTEGER NOT NULL DEFAULT 0, part INTEGER NOT NULL, "
							 "UNIQUE (abode_id, place_id, aspect));"
							 "INSERT INTO rooms (id, abode_id, place_id, aspect, part) VALUES "
							 "(7, 3, 3, 0, 0), (8, 3, 3, 1, 1), (9, 3, 3, 2, 0), (10, 3, 3, 3, 0);",
							 nullptr, nullptr, nullptr)
				== SQLITE_OK);
		sqlite3_close(db);
	}

	SqlitePersistenceBundle bundle(directory);
	const auto rooms = bundle.spatiality().rooms(domain::id::Place{3});
	REQUIRE(rooms.size() == 3);
	CHECK(rooms[0].aspect() == domain::matter::Room::Aspect::Words);
	CHECK(rooms[1].aspect() == domain::matter::Room::Aspect::Threshold);
	CHECK(rooms[1].part() == domain::matter::Room::Part::Outer);
	CHECK(rooms[2].aspect() == domain::matter::Room::Aspect::Dwellers);
}


TEST_CASE("sqlite keeps fatherhoods across reopen; the spiritual father is replaced, the fleshly is not")
{
	using namespace will;
	using namespace will::domain;
	using Line = matter::Fatherhood::Line;

	const std::string directory = "/tmp/will-sqlite-fatherhoods-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

	const DeviceToken token_seth = *DeviceToken::parse("aaaa1234aaaa1234aaaa1234aaaa1234");
	const DeviceToken token_enos = *DeviceToken::parse("bbbb1234bbbb1234bbbb1234bbbb1234");

	std::optional<id::Soul> adam, seth, enos;
	{
		SqlitePersistenceBundle bundle(directory);
		World& world = bundle.world();
		seth = born(world, token_seth).Soul::id();
		enos = born(world, token_enos).Soul::id();
		adam = will::domain::test::forefather(world).Soul::id();

		Eternity& eternity = bundle.eternity();
		CHECK_THROWS_AS(eternity.father(*enos, *seth, Line::Flesh), std::logic_error);
		eternity.father(*adam, *enos, Line::Spirit);
		eternity.father(*seth, *enos, Line::Spirit);
	}

	{
		SqlitePersistenceBundle bundle(directory);
		const std::vector<matter::Fatherhood> kept = bundle.eternity().fatherhoods();
		REQUIRE(kept.size() == 3);
		CHECK(kept[0].father() == adam);
		CHECK(kept[0].child() == seth);
		CHECK(kept[0].line() == Line::Flesh);
		CHECK(kept[1].father() == adam);
		CHECK(kept[1].child() == enos);
		CHECK(kept[1].line() == Line::Flesh);
		CHECK(kept[2].father() == seth);
		CHECK(kept[2].child() == enos);
		CHECK(kept[2].line() == Line::Spirit);

		// The living know their fathers again.
		World& world = bundle.world();
		const Man& living_enos = born(world, token_enos);
		CHECK(living_enos.father(Line::Flesh)->Soul::id() == adam);
		CHECK(living_enos.father(Line::Spirit)->Soul::id() == seth);
	}

}


TEST_CASE("sqlite keeps the exercises of a word across reopen, each approach with its weight and repetitions")
{
	using namespace will;
	using namespace will::domain;

	const std::string directory = "/tmp/will-sqlite-trainings-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

	const id::Word willed{7}, plain{8};
	{
		SqlitePersistenceBundle bundle(directory);
		Eternity& eternity = bundle.eternity();
		eternity.train(willed, {
			Exercise{"Жим лёжа", {Approach{Weight{60'000}, 10, 90}, Approach{Weight{70'000}, 8, 120}, Approach{Weight{62'500}, 6}}},
			Exercise{"Подтягивания", {Approach{Weight{0}, 8}}},
		});
		CHECK_THROWS_AS(eternity.train(willed, {Exercise{"Присед", {Approach{Weight{100'000}, 5}}}}), std::logic_error);
	}

	{
		SqlitePersistenceBundle bundle(directory);
		const std::vector<matter::Training> kept = bundle.eternity().trainings({plain, willed});
		REQUIRE(kept.size() == 1);
		CHECK(kept[0].id() == willed);
		REQUIRE(kept[0].exercises().size() == 2);
		const Exercise& press = kept[0].exercises()[0];
		CHECK(press.name() == "Жим лёжа");
		REQUIRE(press.approaches().size() == 3);
		CHECK(press.approaches()[1] == Approach{Weight{70'000}, 8, 120});
		CHECK(press.approaches()[0].rest_seconds() == 90);
		CHECK(press.approaches()[2].rest_seconds() == 0);
		CHECK(press.approaches()[2].weight().grams() == 62'500);
		CHECK(kept[0].exercises()[1].approaches()[0].weight().own());
	}
}


TEST_CASE("sqlite gives the approaches kept before rest was no rest")
{
	using namespace will;
	using namespace will::domain;

	const std::string directory = "/tmp/will-sqlite-rest-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

	// As kept before approaches had a rest.
	{
		sqlite3* db = nullptr;
		REQUIRE(sqlite3_open((directory + "/will.eternity.db").c_str(), &db) == SQLITE_OK);
		REQUIRE(sqlite3_exec(db,
							 "CREATE TABLE exercises (word_id INTEGER NOT NULL, ord INTEGER NOT NULL, name TEXT NOT NULL, "
							 "PRIMARY KEY (word_id, ord));"
							 "CREATE TABLE approaches (word_id INTEGER NOT NULL, exercise_ord INTEGER NOT NULL, "
							 "ord INTEGER NOT NULL, weight_grams INTEGER NOT NULL, repetitions INTEGER NOT NULL, "
							 "PRIMARY KEY (word_id, exercise_ord, ord));"
							 "INSERT INTO exercises VALUES (5, 0, 'Присед');"
							 "INSERT INTO approaches VALUES (5, 0, 0, 100000, 5);",
							 nullptr, nullptr, nullptr)
				== SQLITE_OK);
		sqlite3_close(db);
	}

	SqlitePersistenceBundle bundle(directory);
	const std::vector<matter::Training> kept = bundle.eternity().trainings({id::Word{5}});
	REQUIRE(kept.size() == 1);
	CHECK(kept[0].exercises()[0].approaches()[0] == Approach{Weight{100'000}, 5, 0});
}


TEST_CASE("sqlite keeps the efforts of a training across reopen; an approach is done once")
{
	using namespace will;
	using namespace will::domain;

	const std::string directory = "/tmp/will-sqlite-efforts-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

	const id::Word training{7};
	Timestamp begun{0};
	{
		SqlitePersistenceBundle bundle(directory);
		Temporality& time = bundle.temporality();
		begun = Timestamp{bundle.eternity().time().instant().value() - 40'000'000'000};  // forty seconds ago
		const matter::Effort first = time.exert(training, 0, 0, Weight{100'000}, 5, begun);
		CHECK(first.finished() >= first.begun());
		time.exert(training, 0, 1, Weight{100'000}, 3, begun);
		CHECK_THROWS_AS(time.exert(training, 0, 0, Weight{100'000}, 5, begun), std::logic_error);
	}

	{
		SqlitePersistenceBundle bundle(directory);
		const std::vector<matter::Effort> kept = bundle.temporality().efforts({training, id::Word{8}});
		REQUIRE(kept.size() == 2);
		CHECK(kept[0].approach() == 0);
		CHECK(kept[1].approach() == 1);
		CHECK(kept[1].repetitions() == 3);
		CHECK(kept[1].weight().grams() == 100'000);
		CHECK(kept[0].begun() == begun);
		CHECK(kept[0].finished() > kept[0].begun());
	}

	CHECK_THROWS_AS((matter::Effort{training, 0, 0, Weight{0}, 5, Timestamp{10}, Timestamp{9}}), std::invalid_argument);
	CHECK_THROWS_AS((matter::Effort{training, 0, 0, Weight{0}, 0, Timestamp{9}, Timestamp{10}}), std::invalid_argument);
}
