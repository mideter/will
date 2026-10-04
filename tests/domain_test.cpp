#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "domain_fakes.h"

#include "acts/creation.h"
#include "immanents/contemplation.h"
#include "immanents/obedience.h"
#include "immanents/shepherding.h"
#include "immanents/tie.h"
#include "matter/man.h"
#include "matter/soul.h"
#include "matter/vessel.h"
#include "matter/tie.h"
#include "words/deed.h"
#include "words/word.h"
#include "properties/immanent.h"
#include "dimensions/eternity.h"
#include "words/letter.h"
#include "immanents/novice.h"
#include "immanents/testator.h"
#include "immanents/witness.h"
#include "values/abode_name.h"
#include "values/device_token.h"
#include "values/timestamp.h"
#include "values/soul_name.h"

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <string>
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


TEST_CASE("welcome creates man with personal abode")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const DeviceToken token = DeviceToken::generate();
	const Man& man = world.welcome(token);
	const auto& witness = static_cast<const Witness&>(man);

	CHECK(man.Soul::id().value() > 0);
	CHECK(man.Vessel::id().value() > 0);
	CHECK(witness.abode().id().value() > 0);
	CHECK(witness.abode().abode_id() == id::Abode{witness.abode().id()});
	CHECK(witness.abode().dwells(man));

	// A man is born asleep: he contemplates nothing until he wakes.
	CHECK_FALSE(world.contemplation(man.Soul::id()));
	CHECK(world.contemplating(man.abode()).empty());
	CHECK_THROWS_AS(man.say("in my sleep"), std::logic_error);

	witness.wake();
	REQUIRE(world.contemplation(man.Soul::id()));
	CHECK(&world.contemplation(man.Soul::id())->abode() == &man.abode());
	CHECK(world.contemplating(man.abode()).size() == 1);
	CHECK(&world.contemplating(man.abode()).front().get() == static_cast<const Soul*>(&man));
	CHECK(world.knows(man.Vessel::id()));
	CHECK(world.knows(man.Soul::id()));

	const Vessel& vessel = world.vessel(man.Vessel::id());
	CHECK(vessel.id() == man.Vessel::id());
	CHECK(world.man(vessel).Soul::id() == man.Soul::id());
	CHECK(&world.man(vessel) == &man);
	CHECK(world.man(vessel).Soul::id() == man.Soul::id());

	const Soul& soul = world.soul(man.Soul::id());
	CHECK(&soul == static_cast<const Soul*>(&man));
	CHECK(SoulName::parse(soul.name().text()));
	CHECK(dynamic_cast<const Testator*>(&man));
	CHECK(dynamic_cast<const Novice*>(&man));
	CHECK(dynamic_cast<const Witness*>(&man));
}


TEST_CASE("welcome existing man")
{
	InMemoryCosmos cosmos;
	seed_man(cosmos.temporality(), id::Soul{42}, test_token("abcd1234abcd1234abcd1234abcd1234"), test_name("oldname1"));
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& man = world.welcome(test_token("abcd1234abcd1234abcd1234abcd1234"));
	CHECK(man.Soul::id() == id::Soul{42});
	CHECK(static_cast<const Witness&>(man).abode().dwells(man));
	CHECK(static_cast<const Witness&>(man).abode().id().value() > 0);
	CHECK(static_cast<const Witness&>(man).abode().id() != id::Place{man.Soul::id().value()});
}


TEST_CASE("welcome keeps existing name")
{
	InMemoryCosmos cosmos;
	seed_man(cosmos.temporality(), id::Soul{7}, test_token("abcd1234abcd1234abcd1234abcd1234"), test_name("keptname"));
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	(void)world.welcome(test_token("abcd1234abcd1234abcd1234abcd1234"));

	CHECK(world.soul(id::Soul{7}).name() == test_name("keptname"));
}


TEST_CASE("each man has a distinct personal abode")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& a = world.welcome(DeviceToken::generate());
	const Man& b = world.welcome(DeviceToken::generate());
	const auto& wa = static_cast<const Witness&>(a);
	const auto& wb = static_cast<const Witness&>(b);

	CHECK(wa.abode().id() != wb.abode().id());
	CHECK(&wa.abode() != &wb.abode());
	CHECK(wa.abode().dwells(a));
	CHECK_FALSE(wa.abode().dwells(b));
	CHECK(wb.abode().dwells(b));
	CHECK_FALSE(wb.abode().dwells(a));

	wa.wake();
	wb.wake();
	CHECK(&world.contemplation(a.Soul::id())->abode() == &a.abode());
	CHECK(&world.contemplation(b.Soul::id())->abode() == &b.abode());
	CHECK(world.contemplating(a.abode()).size() == 1);
	CHECK(world.contemplating(b.abode()).size() == 1);
	CHECK(&world.contemplation(a.Soul::id())->abode() != &b.abode());
	CHECK(&world.contemplating(a.abode()).front().get() == static_cast<const Soul*>(&a));
	CHECK(&world.contemplating(b.abode()).front().get() == static_cast<const Soul*>(&b));
}


TEST_CASE("man say persists via temporality")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& author = world.welcome(DeviceToken::generate());
	const auto& witness = static_cast<const Witness&>(author);
	witness.wake();
	author.say("hello");

	const auto placed = cosmos.spatiality().placements(witness.abode().id(), 10);
	REQUIRE(placed.size() == 1);
	const auto dated = cosmos.temporality().datings({placed[0].id()});
	REQUIRE(dated.size() == 1);
	const auto uttered = cosmos.eternity().word(placed[0].id());
	CHECK(uttered.author() == author.Soul::id());
	CHECK(uttered.saying().body() == "hello");
	CHECK(dated[0].created_at() == Timestamp{1});
}


TEST_CASE("letters of an abode are seen through contemplation by one who dwells in it")
{
	InMemoryCosmos cosmos;
	const id::Soul author{1};

	seed_man(cosmos.temporality(), author, test_token("feedfacefeedfacefeedfacefeedface"), test_name("authoraa"));
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& man = world.welcome(test_token("feedfacefeedfacefeedfacefeedface"));
	const auto& witness = static_cast<const Witness&>(man);
	witness.wake();

	for (int i = 0; i < 5; ++i)
		man.say("m" + std::to_string(i));

	const auto items = world.contemplation(witness.Soul::id())->letters();
	REQUIRE(items.size() == 5);
	CHECK(items.front()->saying().body() == "m0");
	CHECK(items.back()->saying().body() == "m4");
	CHECK(&items.front()->place() == &witness.abode());

	// Waking, a witness contemplates his own abode.
	const auto& stranger = static_cast<const Witness&>(world.welcome(DeviceToken::generate()));
	stranger.wake();
	CHECK(&world.contemplation(stranger.Soul::id())->abode() == &stranger.abode());
	CHECK(world.contemplation(stranger.Soul::id())->letters().empty());

	// A contemplation is living: one gaze per soul, kept by Heaven, never copied.
	static_assert(!std::is_copy_constructible_v<Contemplation>);
	static_assert(!std::is_move_constructible_v<Contemplation>);
	CHECK(world.contemplation(stranger.Soul::id()) == world.contemplation(stranger.Soul::id()));
	CHECK(&world.contemplation(stranger.Soul::id())->who() == &stranger);
	CHECK(world.contemplating(stranger.abode()).size() == 1);

	// One cannot contemplate a place without dwelling in it: the gaze stays where it was.
	CHECK_THROWS_AS(stranger.contemplate(witness.abode()), std::logic_error);
	CHECK(&world.contemplation(stranger.Soul::id())->abode() == &stranger.abode());
	CHECK(world.contemplating(witness.abode()).size() == 1);

	// Turning the gaze to an abode one dwells in ends the former gaze and begins a new one.
	const std::shared_ptr<const Contemplation> former = world.contemplation(stranger.Soul::id());
	stranger.contemplate(stranger.abode());
	CHECK(world.contemplation(stranger.Soul::id()) != former);
	CHECK(world.contemplating(stranger.abode()).size() == 1);

	// An abode shows its letters only to a contemplation of itself.
	CHECK_THROWS_AS(stranger.abode().letters(*world.contemplation(witness.Soul::id())), std::logic_error);

	// A gaze that someone still holds does not fade under him when its soul sleeps.
	const std::shared_ptr<const Contemplation> held = world.contemplation(witness.Soul::id());
	witness.sleep();
	CHECK_FALSE(world.contemplation(witness.Soul::id()));
	CHECK(world.contemplating(witness.abode()).empty());
	CHECK(held->letters().size() == 5);
	CHECK_THROWS_AS(man.say("asleep"), std::logic_error);

	// Sleeping twice is nothing; waking returns him to his own abode.
	witness.sleep();
	stranger.sleep();
	CHECK(world.contemplating(witness.abode()).empty());
	stranger.wake();
	CHECK(&world.contemplation(stranger.Soul::id())->abode() == &stranger.abode());
}


TEST_CASE("matter of a man is a soul, a vessel and the embodiment that joins them")
{
	const matter::Soul soul{id::Soul{3}, test_name("soulname")};
	const matter::Vessel body{id::Vessel{5}, test_token("abcd1234abcd1234abcd1234abcd1234")};
	const matter::Embodiment embodied{id::Soul{3}, id::Vessel{5}};

	const matter::Man man{soul, body, embodied};
	CHECK(man.soul().id() == id::Soul{3});
	CHECK(man.vessel().id() == id::Vessel{5});

	// The embodiment must join exactly this soul to exactly this vessel.
	const matter::Vessel other_body{id::Vessel{6}, test_token("feedfacefeedfacefeedfacefeedface")};
	CHECK_THROWS_AS((matter::Man{soul, other_body, embodied}), std::invalid_argument);
	CHECK_THROWS_AS((matter::Man{soul, body, matter::Embodiment{id::Soul{4}, id::Vessel{5}}}),
					std::invalid_argument);
}


TEST_CASE("a word is immanent to Eternity and is neither copied nor moved in any role")
{
	static_assert(std::is_base_of_v<Immanent<Eternity>, Word>);
	static_assert(!std::is_copy_constructible_v<Letter>);
	static_assert(!std::is_move_constructible_v<Letter>);
	static_assert(!std::is_copy_constructible_v<Deed>);
	static_assert(!std::is_move_constructible_v<Deed>);
}


TEST_CASE("tie is a place for distinct testator and novice")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const auto& testator = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));
	const auto& novice = static_cast<const Novice&>(world.welcome(DeviceToken::generate()));
	const id::Tie oid{7};

	const Tie tie{matter::Tie{oid, testator.Soul::id(), novice.Soul::id()}};
	CHECK(tie.id() == id::Place{oid.value()});
	CHECK(&tie.testator() == &testator);
	CHECK(&tie.novice() == &novice);

	CHECK_THROWS_AS((matter::Tie{id::Tie{8}, testator.Soul::id(), testator.Soul::id()}),
					std::invalid_argument);
}


TEST_CASE("supplicate accept creates tie owned as obedience")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& a = world.welcome(DeviceToken::generate());
	const Man& b = world.welcome(DeviceToken::generate());
	const auto& novice = static_cast<const Novice&>(a);
	const auto& testator = static_cast<const Testator&>(b);

	novice.supplicate(testator);
	CHECK(testator.supplications().size() == 1);
	const Supplication& ask = testator.supplication(novice);
	CHECK(ask.suppliant().Soul::id() == a.Soul::id());
	CHECK(ask.addressee().Soul::id() == b.Soul::id());
	CHECK(cosmos.temporality().supplications(b.Soul::id()).size() == 1);

	const Shepherding& shepherding = testator.accept(ask);
	CHECK(testator.supplications().empty());
	REQUIRE(cosmos.spatiality().ties().size() == 1);
	CHECK(id::Tie{shepherding.id()} == cosmos.spatiality().ties().front().id());
	CHECK(&testator.shepherding(novice) == &shepherding);
	CHECK_THROWS_AS(static_cast<const Testator&>(a).shepherding(novice), std::invalid_argument);
	const Obedience& obedience = novice.obedience(testator);
	CHECK(&obedience.testator() == &testator);
	CHECK(&obedience.novice() == &novice);
	CHECK(obedience.id() == shepherding.id());
	CHECK(obedience.id().value() != a.Soul::id().value());
	CHECK(obedience.id().value() != b.Soul::id().value());
	CHECK(novice.follows(testator));
	CHECK_FALSE(testator.follows(static_cast<const Testator&>(a)));
	CHECK_THROWS_AS(testator.obedience(static_cast<const Testator&>(a)), std::invalid_argument);

	// What is kept decides: the pair is tied, so neither a new supplication nor a new bond is kept.
	CHECK_THROWS_AS(novice.supplicate(testator), std::logic_error);
	CHECK_THROWS_AS(cosmos.spatiality().bind(b.Soul::id(), a.Soul::id()), std::logic_error);
	CHECK(cosmos.spatiality().ties().size() == 1);
}


TEST_CASE("reject closes pending supplication; wrong party cannot accept")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& a = world.welcome(DeviceToken::generate());
	const Man& b = world.welcome(DeviceToken::generate());
	const auto& novice = static_cast<const Novice&>(a);
	const auto& testator = static_cast<const Testator&>(b);
	const auto& stranger = static_cast<const Testator&>(a);

	novice.supplicate(testator);
	const Supplication& ask = testator.supplication(novice);
	CHECK_THROWS_AS(stranger.accept(ask), std::logic_error);

	testator.reject(ask);
	CHECK(testator.supplications().empty());
	CHECK(cosmos.temporality().supplications(b.Soul::id()).empty());
	CHECK_THROWS_AS(testator.supplication(novice), std::invalid_argument);
	CHECK_THROWS_AS(cosmos.temporality().reject(a.Soul::id(), b.Soul::id()), std::invalid_argument);
	CHECK(cosmos.spatiality().ties().empty());

	// A rejected suppliant may ask again; a second supplication while one awaits is refused.
	novice.supplicate(testator);
	CHECK(cosmos.temporality().supplications(b.Soul::id()).size() == 1);
	CHECK_THROWS_AS(novice.supplicate(testator), std::logic_error);
	CHECK_THROWS_AS(novice.supplicate(static_cast<const Testator&>(a)), std::invalid_argument);
}


TEST_CASE("will and execute within obedience")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& a = world.welcome(DeviceToken::generate());
	const Man& b = world.welcome(DeviceToken::generate());
	CHECK(&world.man(a.name()) == &a);
	CHECK(&world.man(b.name()) == &b);
	CHECK_THROWS_AS(world.man(*SoulName::parse("zzzzzzzz")), std::invalid_argument);

	const auto& novice = static_cast<const Novice&>(a);
	const auto& testator = static_cast<const Testator&>(b);

	novice.supplicate(testator);
	testator.accept(testator.supplication(novice));
	REQUIRE(cosmos.spatiality().ties().size() == 1);
	const Shepherding& shepherding = testator.shepherding(novice);
	const Deed deed = testator.will(shepherding, "fast");
	CHECK(deed.open());
	CHECK(deed.saying().body() == "fast");
	CHECK(&deed.tie().testator() == &testator);
	CHECK(&deed.tie().novice() == &novice);

	CHECK_THROWS_AS(static_cast<const Testator&>(a).will(shepherding, "no"), std::logic_error);

	// The tie shows its deeds to either side, through the one who asks.
	const Obedience& obedience = novice.obedience(testator);
	REQUIRE(obedience.deeds(novice).size() == 1);
	CHECK(obedience.deeds(novice).front()->open());
	REQUIRE(shepherding.deeds(testator).size() == 1);
	CHECK(shepherding.deeds(testator).front()->id() == deed.id());
	CHECK_THROWS_AS(testator.execute(deed), std::logic_error);

	novice.execute(deed);
	const std::shared_ptr<const Deed> done = obedience.deeds(novice).front();
	CHECK(done->executed());
	CHECK_FALSE(done->open());
	CHECK(shepherding.deeds(testator).front()->executed());
	CHECK_THROWS_AS(novice.execute(*done), std::logic_error);

	// Stale open snapshot: what is kept decides, not the argument.
	CHECK_THROWS_AS(novice.execute(deed), std::logic_error);
	CHECK(cosmos.temporality().executions({deed.id()}).size() == 1);
}


TEST_CASE("a tie shows its deeds only to its sides; deeds are not letters")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const auto& novice = static_cast<const Novice&>(world.welcome(DeviceToken::generate()));
	const auto& testator = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));
	const auto& stranger = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));

	novice.supplicate(testator);
	testator.accept(testator.supplication(novice));
	const Deed deed = testator.will(testator.shepherding(novice), "fast");

	novice.wake();
	testator.wake();

	const Obedience& obedience = novice.obedience(testator);
	CHECK_THROWS_AS(obedience.deeds(stranger), std::logic_error);
	CHECK(novice.obediences().size() == 1);
	CHECK(&novice.obediences().front().get() == &obedience);
	CHECK(stranger.obediences().empty());

	// A deed is placed in its tie, not in the abodes of its sides.
	CHECK(world.contemplation(testator.Soul::id())->letters().empty());
	CHECK(world.contemplation(novice.Soul::id())->letters().empty());

	// A letter is not a deed.
	static_cast<const Witness&>(novice).say("hello");
	REQUIRE(world.contemplation(novice.Soul::id())->letters().size() == 1);
	REQUIRE(obedience.deeds(novice).size() == 1);
	CHECK(obedience.deeds(novice).front()->id() == deed.id());
}
