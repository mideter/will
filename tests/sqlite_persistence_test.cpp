#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "horizons/creation.h"
#include "words/behest.h"
#include "words/deed.h"
#include "places/obedience.h"
#include "words/letter.h"
#include "men/man.h"
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


std::vector<std::shared_ptr<const will::domain::Letter>>
letters_seen_by(const std::shared_ptr<const will::domain::Contemplation>& gaze)
{
	std::vector<std::shared_ptr<const will::domain::Letter>> letters;
	for (const std::shared_ptr<const will::domain::Word>& word : gaze->room().words(*gaze)) {
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
using will::domain::test::regard_in_helm;
using will::domain::test::born;
using will::domain::test::supplicate;
using will::domain::test::accept;
using will::domain::test::gaze_of;
using will::domain::test::reject;

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
		abode_a_place = man_a.abode().id();

		man_a.wake();
		man_b.wake();
		man_a.contemplate(*man_a.abode().room(man_a.abode()));
		man_b.contemplate(*man_b.abode().room(man_b.abode()));
		{ const auto gaze = world.contemplation(man_a); gaze->room().write(*gaze, "from-peer"); }
		{ const auto gaze = world.contemplation(man_b); gaze->room().write(*gaze, "from-me"); }

		const auto& witness_a = man_a;
		const auto letters = letters_seen_by(world.contemplation(witness_a));
		REQUIRE(letters.size() == 1);
		CHECK(letters[0]->saying().body() == "from-peer");
		CHECK(letters[0]->author().id() == man_a.Soul::id());
		CHECK(world.soul(letters[0]->author().id()).name() == *name_a);

		// man_b spoke in his own abode; check via spatiality/eternity on place_a only has man_a
		const auto placed = bundle.spatiality().placements(witness_a.abode().id());
		REQUIRE(placed.size() == 1);

		CHECK(man_a.abode().id() !=
			  man_b.abode().id());

		// One abode per soul: what is kept decides.
		REQUIRE(bundle.spatiality().abode(man_a.Soul::id()));
		CHECK(id::Place{bundle.spatiality().abode(man_a.Soul::id())->id().value()} == *abode_a_place);
		CHECK_THROWS_AS(bundle.spatiality().abide(man_a.Soul::id()),
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
		CHECK(man_a_reloaded.abode().dwells(man_a_reloaded));
		CHECK(man_a_reloaded.abode().id() == *abode_a_place);

		// After the World awakens anew everyone is asleep until he comes.
		CHECK_FALSE(world.contemplation(man_a_reloaded));
		man_a_reloaded.wake();
		man_a_reloaded.contemplate(*man_a_reloaded.abode().room(man_a_reloaded.abode()));

		const auto letters = letters_seen_by(world.contemplation(man_a_reloaded));
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


TEST_CASE("sqlite keeps a behest, its execution and its review across reopen")
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

		const auto& novice = born(world, token_novice);
		const auto& testator = born(world, token_testator);

		admit_at_gates(world, novice, testator);
		admit_at_gates(world, testator, novice);
		supplicate(world, novice, testator);
		accept(world, testator, *testator.supplication(novice));
		testator.wake();
		testator.contemplate(*testator.abode().room(testator.shepherding(novice)));
		novice.wake();
		novice.contemplate(*novice.abode().room(novice.obedience(testator)));

		const std::shared_ptr<const Behest> open_held = std::dynamic_pointer_cast<const Behest>(world.contemplation(testator)->room().write(*world.contemplation(testator), "fast"));
		const Behest& open = *open_held;
		const std::shared_ptr<const Behest> willed_held = std::dynamic_pointer_cast<const Behest>(world.contemplation(testator)->room().write(*world.contemplation(testator), "pray"));
		const Behest& willed = *willed_held;
		CHECK(&willed.tie().novice() == &novice);
		open_id = open.id();
		done_id = willed.id();

		test::execute(world, novice, willed);
		CHECK_THROWS_AS(test::execute(world, novice, willed), std::logic_error);
		CHECK(bundle.temporality().executions({*open_id, *done_id}).size() == 1);
		// Done before the testator's eyes, the deed is reviewed at once, and once.
		CHECK(testator.unreviewed().empty());
		const id::Word deed_id = novice.obedience(testator).deeds(gaze_of(world, novice)).front()->id();
		REQUIRE(bundle.temporality().reviews({deed_id}).size() == 1);
		CHECK_THROWS_AS(bundle.temporality().review(deed_id), std::logic_error);

		// Behests are placed in the tie, not among the letters of an abode.
		novice.contemplate(will::domain::test::room_of(novice));
		CHECK(letters_seen_by(world.contemplation(novice)).empty());
	}

	{
		SqlitePersistenceBundle bundle(directory);
		World& world = bundle.world();

		const auto& novice = born(world, token_novice);
		const auto& testator = born(world, token_testator);

		// The reborn tie shows the same behests to both sides, oldest first; the novice acts
		// on them as he beholds them, standing in his room of the tie.
		novice.wake();
		novice.contemplate(*novice.abode().room(novice.obedience(testator)));
		const std::vector<std::shared_ptr<const Behest>> shown = novice.obedience(testator).behests(gaze_of(world, novice));
		REQUIRE(shown.size() == 2);
		const Behest& open = *shown[0];
		const Behest& done = *shown[1];

		CHECK(open.id() == *open_id);
		CHECK(open.saying().body() == "fast");
		CHECK(&open.tie().testator() == &testator);

		CHECK(done.id() == *done_id);
		CHECK(done.saying().body() == "pray");
		CHECK(testator.shepherding(novice).behests(gaze_of(world, novice)).size() == 2);

		// The deed fulfilling «pray» was kept, and is born again with it.
		const auto deeds = novice.obedience(testator).deeds(gaze_of(world, novice));
		REQUIRE(deeds.size() == 1);
		CHECK(deeds.front()->behest().id() == *done_id);
		CHECK(deeds.front()->saying().body() == "совершено");
		CHECK(deeds.front()->review());
		CHECK(done.deed() == deeds.front());
		CHECK_FALSE(open.deed());
		CHECK_THROWS_AS(test::execute(world, novice, done), std::logic_error);

		test::execute(world, novice, open);
		CHECK(novice.obedience(testator).deeds(gaze_of(world, novice)).size() == 2);
		// Done while the testator sleeps, it awaits his review.
		REQUIRE(testator.unreviewed().size() == 1);
		CHECK(testator.unreviewed()[0]->id() == *open_id);
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

		const auto& a = born(world, token_a);
		const auto& b = born(world, token_b);
		const auto& c = born(world, token_c);

		// a asks b twice: rejected, then asked again and left awaiting.
		admit_at_gates(world, a, b);
		admit_at_gates(world, b, a);
		supplicate(world, a, b);
		CHECK_THROWS_AS(supplicate(world, a, b), std::logic_error);
		reject(world, b, *b.supplication(a));
		CHECK(bundle.temporality().supplications(b.Soul::id()).empty());
		CHECK_THROWS_AS(bundle.temporality().answer(a.Soul::id(), b.Soul::id(), matter::Answer::Form::Rejected),
						std::invalid_argument);
		supplicate(world, a, b);

		// c asks b and is accepted.
		admit_at_gates(world, c, b);
		admit_at_gates(world, b, c);
		supplicate(world, c, b);
		CHECK(bundle.temporality().supplications(b.Soul::id()).size() == 2);
		const Shepherding& shepherding = accept(world, b, *b.supplication(c));
		CHECK(&shepherding.novice() == &c);
		CHECK(b.supplications().size() == 1);
		CHECK_THROWS_AS(supplicate(world, c, b), std::logic_error);
	}

	{
		SqlitePersistenceBundle bundle(directory);
		World& world = bundle.world();

		const auto& a = born(world, token_a);
		const auto& b = born(world, token_b);
		const auto& c = born(world, token_c);

		// The awaiting supplication is reborn on the addressee's heap; the accepted
		// one is answered and stays behind; the tie is living.
		CHECK(bundle.temporality().supplications(b.Soul::id()).size() == 1);
		REQUIRE(b.supplications().size() == 1);
		CHECK(&b.supplication(a)->suppliant() == &a);
		CHECK(&b.shepherding(c).novice() == &c);
		CHECK_THROWS_AS(b.shepherding(a), std::invalid_argument);

		const Shepherding& shepherding = accept(world, b, *b.supplication(a));
		CHECK(&shepherding.novice() == &a);
		CHECK(b.supplications().empty());
		CHECK(bundle.spatiality().ties().size() == 2);
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
		regard_in_helm(world, host, man, matter::Dweller::Kind::Friend);
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

		// Each abode keeps its own room across reopen, and its gates and helm with it.
		REQUIRE(host.abode().rooms().size() == 1);
		CHECK(&host.abode().rooms()[0].get().reflects() == static_cast<const Place*>(&host.abode()));
		CHECK(bundle.spatiality().rooms(host.abode().id()).size() == 1);
		CHECK(host.abode().outer().gates().id() == bundle.spatiality().abode(host.Soul::id())->gates());
		CHECK(host.abode().inner().helm().id() == bundle.spatiality().abode(host.Soul::id())->helm());
	}

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
		eternity.utter(willed, {
			Exercise{"Жим лёжа", {Approach{Weight{60'000}, 10, 90}, Approach{Weight{70'000}, 8, 120}, Approach{Weight{62'500}, 6}},
					 "Лопатки сведены"},
			Exercise{"Подтягивания", {Approach{Weight{0}, 8}}},
		});
		CHECK_THROWS_AS(eternity.utter(willed, {Exercise{"Присед", {Approach{Weight{100'000}, 5}}}}), std::logic_error);
	}

	{
		SqlitePersistenceBundle bundle(directory);
		const std::vector<matter::Training> kept = bundle.eternity().trainings({plain, willed});
		REQUIRE(kept.size() == 1);
		CHECK(kept[0].id() == willed);
		REQUIRE(kept[0].exercises().size() == 2);
		const Exercise& press = kept[0].exercises()[0];
		CHECK(press.name() == "Жим лёжа");
		CHECK(press.instruction() == "Лопатки сведены");
		CHECK(kept[0].exercises()[1].instruction().empty());
		REQUIRE(press.approaches().size() == 3);
		CHECK(press.approaches()[1] == Approach{Weight{70'000}, 8, 120});
		CHECK(press.approaches()[0].rest_seconds() == 90);
		CHECK(press.approaches()[2].rest_seconds() == 0);
		CHECK(press.approaches()[2].weight().grams() == 62'500);
		CHECK(kept[0].exercises()[1].approaches()[0].weight().own());
	}
}


TEST_CASE("sqlite keeps the remarks on the exercises of a training across reopen; a word is a remark once")
{
	using namespace will;
	using namespace will::domain;

	const std::string directory = "/tmp/will-sqlite-remarks-test-" + std::to_string(getpid());
	const ScratchDirectory scratch{directory};

	const id::Word training{7}, other{8}, knee{9}, passed{10};
	{
		SqlitePersistenceBundle bundle(directory);
		Eternity& eternity = bundle.eternity();
		eternity.remark(knee, training, 0);
		eternity.remark(passed, training, 1);
		CHECK_THROWS(eternity.remark(knee, training, 1));
	}

	SqlitePersistenceBundle bundle(directory);
	const std::vector<matter::Remarked> kept = bundle.eternity().remarked({other, training});
	REQUIRE(kept.size() == 2);
	CHECK(kept[0].id() == knee);
	CHECK(kept[0].exercise() == 0);
	CHECK(kept[1].id() == passed);
	CHECK(kept[1].training() == training);
	CHECK(kept[1].exercise() == 1);
}


TEST_CASE("sqlite keeps the approach underway and the efforts of a training across reopen; an approach is done once")
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
		begun = time.begin(training, 0, 0).begun();
		CHECK_THROWS_AS(time.begin(training, 0, 1), std::logic_error);
	}

	{
		// The approach underway outlives the reopen; done, it is no longer underway.
		SqlitePersistenceBundle bundle(directory);
		Temporality& time = bundle.temporality();
		const std::vector<matter::Underway> underway = time.underways({training, id::Word{8}});
		REQUIRE(underway.size() == 1);
		CHECK(underway[0].approach() == 0);
		CHECK(underway[0].begun() == begun);
		const matter::Effort first = time.exert(underway[0], Weight{100'000}, 5, Strain{});
		CHECK(first.begun() == begun);
		CHECK(first.finished() >= first.begun());
		CHECK(time.underways({training}).empty());
		CHECK_THROWS_AS(time.exert(underway[0], Weight{100'000}, 5, Strain{}), std::logic_error);
		time.exert(time.begin(training, 0, 1), Weight{100'000}, 3, Strain{2});
	}

	{
		SqlitePersistenceBundle bundle(directory);
		const std::vector<matter::Effort> kept = bundle.temporality().efforts({training, id::Word{8}});
		REQUIRE(kept.size() == 2);
		CHECK(kept[0].approach() == 0);
		CHECK(kept[1].approach() == 1);
		CHECK(kept[1].repetitions() == 3);
		CHECK(kept[1].strain() == Strain{2});
		CHECK(kept[0].strain() == Strain{});
		CHECK(kept[1].weight().grams() == 100'000);
		CHECK(kept[0].begun() == begun);
		CHECK(kept[0].finished() >= kept[0].begun());
	}

	// An approach ended apart outlives the reopen ended; told, it finished when it ended.
	Timestamp ended{0};
	{
		SqlitePersistenceBundle bundle(directory);
		Temporality& time = bundle.temporality();
		const matter::Underway doing = time.begin(training, 0, 2);
		ended = *time.end(doing).ended();
		CHECK_THROWS_AS(time.end(doing), std::logic_error);
	}
	{
		SqlitePersistenceBundle bundle(directory);
		Temporality& time = bundle.temporality();
		const std::vector<matter::Underway> underway = time.underways({training});
		REQUIRE(underway.size() == 1);
		REQUIRE(underway[0].ended());
		CHECK(*underway[0].ended() == ended);
		CHECK(time.exert(underway[0], Weight{100'000}, 4, Strain{1}).finished() == ended);
	}

	CHECK_THROWS_AS((matter::Effort{training, 0, 0, Weight{0}, 5, Timestamp{10}, Timestamp{9}}), std::invalid_argument);
	CHECK_THROWS_AS((matter::Effort{training, 0, 0, Weight{0}, 0, Timestamp{9}, Timestamp{10}}), std::invalid_argument);
}
