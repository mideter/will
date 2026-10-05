#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <optional>
#include <doctest/doctest.h>

#include "domain_fakes.h"

#include "acts/creation.h"
#include "matter/abode.h"
#include "matter/supplication.h"
#include "acts/supplication.h"
#include "immanents/vessel.h"
#include "properties/birth.h"
#include "dimensions/temporality.h"
#include "dimensions/spatiality.h"
#include "horizons/life.h"
#include "immanents/contemplation.h"
#include "immanents/obedience.h"
#include "immanents/shepherding.h"
#include "immanents/tie.h"
#include "matter/letter.h"
#include "matter/word.h"
#include "matter/placement.h"
#include "matter/dating.h"
#include "matter/man.h"
#include "matter/soul.h"
#include "matter/vessel.h"
#include "matter/tie.h"
#include "words/behest.h"
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
	Creation& creation = cosmos.life().create();
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
	CHECK(&world.contemplation(man.Soul::id())->place() == &man.abode());
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
	seed_man(cosmos, id::Soul{42}, test_token("abcd1234abcd1234abcd1234abcd1234"), test_name("oldname1"));
	Creation& creation = cosmos.life().create();
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
	seed_man(cosmos, id::Soul{7}, test_token("abcd1234abcd1234abcd1234abcd1234"), test_name("keptname"));
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	(void)world.welcome(test_token("abcd1234abcd1234abcd1234abcd1234"));

	CHECK(world.soul(id::Soul{7}).name() == test_name("keptname"));
}


TEST_CASE("each man has a distinct personal abode")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
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
	CHECK(&world.contemplation(a.Soul::id())->place() == &a.abode());
	CHECK(&world.contemplation(b.Soul::id())->place() == &b.abode());
	CHECK(world.contemplating(a.abode()).size() == 1);
	CHECK(world.contemplating(b.abode()).size() == 1);
	CHECK(&world.contemplation(a.Soul::id())->place() != &b.abode());
	CHECK(&world.contemplating(a.abode()).front().get() == static_cast<const Soul*>(&a));
	CHECK(&world.contemplating(b.abode()).front().get() == static_cast<const Soul*>(&b));
}


TEST_CASE("man say persists via temporality")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
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

	seed_man(cosmos, author, test_token("feedfacefeedfacefeedfacefeedface"), test_name("authoraa"));
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const Man& man = world.welcome(test_token("feedfacefeedfacefeedfacefeedface"));
	const auto& witness = static_cast<const Witness&>(man);
	witness.wake();

	for (int i = 0; i < 5; ++i)
		man.say("m" + std::to_string(i));

	const auto items = letters_seen_by(world.contemplation(witness.Soul::id()));
	REQUIRE(items.size() == 5);
	CHECK(items.front()->saying().body() == "m0");
	CHECK(items.back()->saying().body() == "m4");
	CHECK(&items.front()->place() == &witness.abode());

	// Waking, a witness contemplates his own abode.
	const auto& stranger = static_cast<const Witness&>(world.welcome(DeviceToken::generate()));
	stranger.wake();
	CHECK(&world.contemplation(stranger.Soul::id())->place() == &stranger.abode());
	CHECK(letters_seen_by(world.contemplation(stranger.Soul::id())).empty());

	// A contemplation is living: one gaze per soul, kept by Heaven, never copied.
	static_assert(!std::is_copy_constructible_v<Contemplation>);
	static_assert(!std::is_move_constructible_v<Contemplation>);
	CHECK(world.contemplation(stranger.Soul::id()) == world.contemplation(stranger.Soul::id()));
	CHECK(&world.contemplation(stranger.Soul::id())->who() == &stranger);
	CHECK(world.contemplating(stranger.abode()).size() == 1);

	// One cannot contemplate a place without dwelling in it: the gaze stays where it was.
	CHECK_THROWS_AS(stranger.contemplate(witness.abode()), std::logic_error);
	CHECK(&world.contemplation(stranger.Soul::id())->place() == &stranger.abode());
	CHECK(world.contemplating(witness.abode()).size() == 1);

	// Turning the gaze to an abode one dwells in ends the former gaze and begins a new one.
	const std::shared_ptr<const Contemplation> former = world.contemplation(stranger.Soul::id());
	stranger.contemplate(stranger.abode());
	CHECK(world.contemplation(stranger.Soul::id()) != former);
	CHECK(world.contemplating(stranger.abode()).size() == 1);

	// An abode shows its letters only to a contemplation of itself.
	CHECK_THROWS_AS(stranger.abode().words(*world.contemplation(witness.Soul::id())), std::logic_error);

	// A gaze that someone still holds does not fade under him when its soul sleeps.
	const std::shared_ptr<const Contemplation> held = world.contemplation(witness.Soul::id());
	witness.sleep();
	CHECK_FALSE(world.contemplation(witness.Soul::id()));
	CHECK(world.contemplating(witness.abode()).empty());
	CHECK(letters_seen_by(held).size() == 5);
	CHECK_THROWS_AS(man.say("asleep"), std::logic_error);

	// Sleeping twice is nothing; waking returns him to his own abode.
	witness.sleep();
	stranger.sleep();
	CHECK(world.contemplating(witness.abode()).empty());
	stranger.wake();
	CHECK(&world.contemplation(stranger.Soul::id())->place() == &stranger.abode());
}


TEST_CASE("letters of an abode live while it is contemplated, shared by every gaze")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const auto& host = static_cast<const Witness&>(world.welcome(DeviceToken::generate()));

	// Asleep, nobody beholds the abode: nothing is read from the dimensions.
	CHECK(cosmos.spatiality().placement_reads() == 0);

	// The first gaze births the letters from what the dimensions keep.
	host.wake();
	CHECK(cosmos.spatiality().placement_reads() == 1);
	CHECK(letters_seen_by(world.contemplation(host.Soul::id())).empty());

	// A letter said is born living at once and beheld, without reading the dimensions again.
	host.say("first");
	host.say("second");
	const auto seen = letters_seen_by(world.contemplation(host.Soul::id()));
	REQUIRE(seen.size() == 2);
	CHECK(seen[0]->saying().body() == "first");
	CHECK(seen[1]->saying().body() == "second");
	CHECK(&seen[0]->author() == static_cast<const Soul*>(&host));
	CHECK(cosmos.spatiality().placement_reads() == 1);

	// A new gaze upon the same abode shares the very same letters: nothing is read anew.
	const std::shared_ptr<const Contemplation> former = world.contemplation(host.Soul::id());
	host.contemplate(host.abode());
	const std::shared_ptr<const Contemplation> current = world.contemplation(host.Soul::id());
	REQUIRE(current != former);
	CHECK(letters_seen_by(current)[0] == seen[0]);
	CHECK(letters_seen_by(current)[1] == seen[1]);
	CHECK(letters_seen_by(former)[0] == seen[0]);
	CHECK(cosmos.spatiality().placement_reads() == 1);

	// Only the gazes resting on the abode now behold what is said next.
	host.say("third");
	CHECK(letters_seen_by(current).size() == 3);
	CHECK(letters_seen_by(former).size() == 2);

	// A gaze beholds only letters said in what it contemplates.
	const auto& other = static_cast<const Witness&>(world.welcome(DeviceToken::generate()));
	other.wake();
	other.say("elsewhere");
	CHECK_THROWS_AS(current->behold(letters_seen_by(world.contemplation(other.Soul::id())).front()),
					std::logic_error);
	CHECK(letters_seen_by(current).size() == 3);

	// Only its author inscribes a letter, and only into the abode he contemplates.
	const auto stray = matter::Letter{matter::Word{id::Word{900}, other.Soul::id(), "not his"},
									  matter::Placement{id::Word{900}, host.abode().id()},
									  matter::Dating{id::Word{900}, Timestamp{1}}};
	CHECK_THROWS_AS(host.abode().inscribe(*current, stray), std::logic_error);
	CHECK_THROWS_AS(other.abode().inscribe(*current, stray), std::logic_error);
}


TEST_CASE("Life remembers the living letters, so a new gaze shares those still held")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const auto& host = static_cast<const Witness&>(world.welcome(DeviceToken::generate()));

	host.wake();
	host.say("first");
	host.say("second");

	// An old gaze is still held while the man turns anew and says one more.
	const std::shared_ptr<const Contemplation> old_gaze = world.contemplation(host.Soul::id());
	host.contemplate(host.abode());
	host.say("third");
	const std::weak_ptr<const Letter> third = letters_seen_by(world.contemplation(host.Soul::id())).back();

	// He sleeps: the third letter is held by no one and is gone; the first two live in the old gaze.
	host.sleep();
	CHECK(third.expired());
	REQUIRE(letters_seen_by(old_gaze).size() == 2);

	// Waking, he beholds all three: the two still living are the very same, the third is born anew.
	const std::size_t reads = cosmos.spatiality().placement_reads();
	host.wake();
	const auto seen = letters_seen_by(world.contemplation(host.Soul::id()));
	REQUIRE(seen.size() == 3);
	CHECK(seen[0] == letters_seen_by(old_gaze)[0]);
	CHECK(seen[1] == letters_seen_by(old_gaze)[1]);
	CHECK(seen[2]->saying().body() == "third");
	CHECK(cosmos.spatiality().placement_reads() == reads + 1);
}


TEST_CASE("letters of an abode are gone with the last gaze and are born anew from matter")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const auto& host = static_cast<const Witness&>(world.welcome(DeviceToken::generate()));

	host.wake();
	host.say("kept");
	std::weak_ptr<const Letter> watched = letters_seen_by(world.contemplation(host.Soul::id())).front();
	REQUIRE(watched.lock());
	CHECK(cosmos.spatiality().placement_reads() == 1);

	// While anyone still holds a gaze, its letters live on though the man sleeps.
	std::shared_ptr<const Contemplation> held = world.contemplation(host.Soul::id());
	host.sleep();
	CHECK_FALSE(watched.expired());

	// The last gaze is let go: the living letter is gone, its matter stays kept.
	held.reset();
	CHECK(watched.expired());
	CHECK(cosmos.spatiality().placements(host.abode().id(), 10).size() == 1);
	const std::size_t reads = cosmos.spatiality().placement_reads();

	// A new gaze births the letters anew from the dimensions.
	host.wake();
	CHECK(cosmos.spatiality().placement_reads() == reads + 1);
	const auto reborn = letters_seen_by(world.contemplation(host.Soul::id()));
	REQUIRE(reborn.size() == 1);
	CHECK(reborn.front()->saying().body() == "kept");
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


TEST_CASE("the living is born only of the living")
{
	// No one but the parent can make the key of birth.
	static_assert(!std::is_default_constructible_v<Birth<Abode>>);
	static_assert(!std::is_default_constructible_v<Birth<World>>);
	static_assert(!std::is_constructible_v<Birth<Abode>, const Abode&>);

	// And without the key nothing living is brought forth from its matter.
	static_assert(!std::is_constructible_v<Testator, matter::Man>);
	static_assert(!std::is_constructible_v<Vessel, matter::Vessel>);
	static_assert(!std::is_constructible_v<Abode, matter::Abode>);
	static_assert(!std::is_constructible_v<Tie, matter::Tie>);
	static_assert(!std::is_constructible_v<Supplication, matter::Supplication>);

	// A supplication is living too: a relation of souls, immanent to Heaven.
	static_assert(std::is_base_of_v<Immanent<Heaven>, Supplication>);
	static_assert(!std::is_copy_constructible_v<Supplication>);
	static_assert(!std::is_move_constructible_v<Supplication>);
	static_assert(!std::is_constructible_v<Letter, matter::Letter>);
	static_assert(!std::is_constructible_v<Behest, matter::Behest>);
	static_assert(!std::is_constructible_v<Contemplation, const Witness&, const Abode&>);
}


TEST_CASE("only Life creates the World; it proceeds from Eternity, and there is one Life")
{
	static_assert(!std::is_constructible_v<Birth<Life>, const Life&>);
	static_assert(!std::is_constructible_v<Creation, Eternity&>);
	static_assert(std::is_base_of_v<Immanent<Life>, Creation>);
	static_assert(!std::is_copy_constructible_v<Creation>);
	static_assert(!std::is_move_constructible_v<Creation>);
	static_assert(std::is_base_of_v<Immanent<Eternity>, Life>);
	static_assert(!std::is_copy_constructible_v<Life>);
	static_assert(!std::is_move_constructible_v<Life>);

	InMemoryCosmos cosmos;
	CHECK_THROWS_AS(Life{}, std::logic_error);

	Creation& creation = cosmos.life().create();
	const Man& man = creation.world().welcome(DeviceToken::generate());
	CHECK(creation.world().knows(man.Soul::id()));
}


TEST_CASE("there is one Creation in Life, and the matter of its dimensions outlives Life")
{
	InMemoryShared matter;
	const DeviceToken token = DeviceToken::generate();
	std::optional<id::Soul> soul_id;
	{
		InMemoryCosmos cosmos(matter);
		Creation& creation = cosmos.life().create();
		soul_id = creation.world().welcome(token).Soul::id();
		CHECK_THROWS_AS(cosmos.life().create(), std::logic_error);
		CHECK(&cosmos.life().creation() == &creation);
	}

	InMemoryCosmos cosmos(matter);
	CHECK_THROWS_AS(cosmos.life().creation(), std::logic_error);
	Creation& creation = cosmos.life().create();
	CHECK(creation.world().knows(*soul_id));
	CHECK(creation.world().welcome(token).Soul::id() == *soul_id);
}


TEST_CASE("a word is immanent to Life and is neither copied nor moved in any role")
{
	static_assert(std::is_base_of_v<Immanent<Life>, Word>);
	static_assert(!std::is_copy_constructible_v<Letter>);
	static_assert(!std::is_move_constructible_v<Letter>);
	static_assert(!std::is_copy_constructible_v<Behest>);
	static_assert(!std::is_move_constructible_v<Behest>);
}


TEST_CASE("the sides of a tie may contemplate it and behold its behests")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const auto& novice = static_cast<const Novice&>(world.welcome(DeviceToken::generate()));
	const auto& testator = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));
	const auto& stranger = static_cast<const Witness&>(world.welcome(DeviceToken::generate()));

	novice.supplicate(testator);
	const Shepherding& shepherding = testator.accept(*testator.supplication(novice));
	const std::shared_ptr<const Behest> willed_held = testator.will(shepherding, "fast");
	const Behest& willed = *willed_held;

	novice.wake();
	testator.wake();
	stranger.wake();

	// Both sides dwell in the tie; a stranger does not and cannot turn his gaze to it.
	CHECK(shepherding.dwells(novice));
	CHECK(shepherding.dwells(testator));
	CHECK_FALSE(shepherding.dwells(stranger));
	CHECK_THROWS_AS(stranger.contemplate(shepherding), std::logic_error);

	// Turning to the tie, a side beholds its behests as words.
	testator.contemplate(shepherding);
	const std::shared_ptr<const Contemplation> gaze = world.contemplation(testator.Soul::id());
	CHECK(&gaze->place() == static_cast<const Place*>(&shepherding));
	REQUIRE(gaze->words().size() == 1);
	const auto behest = std::dynamic_pointer_cast<const Behest>(gaze->words().front());
	REQUIRE(behest);
	CHECK(behest->id() == willed.id());
	CHECK(world.contemplating(shepherding).size() == 1);

	// In a tie one wills, one does not say.
	CHECK_THROWS_AS(static_cast<const Man&>(testator).say("hello"), std::logic_error);

	// Turning back, he contemplates his abode again.
	testator.contemplate(testator.abode());
	CHECK(world.contemplating(shepherding).empty());
	static_cast<const Man&>(testator).say("hello");
}


TEST_CASE("the words of a tie live while it is contemplated, one for both sides")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const auto& novice = static_cast<const Novice&>(world.welcome(DeviceToken::generate()));
	const auto& testator = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));

	novice.supplicate(testator);
	const Shepherding& shepherding = testator.accept(*testator.supplication(novice));
	const Obedience& obedience = novice.obedience(testator);

	// Both sides turn to the tie: the testator's gaze births its words, the novice's shares them.
	novice.wake();
	testator.wake();
	testator.contemplate(shepherding);
	novice.contemplate(obedience);
	const std::shared_ptr<const Contemplation> testator_gaze = world.contemplation(testator.Soul::id());
	const std::shared_ptr<const Contemplation> novice_gaze = world.contemplation(novice.Soul::id());
	CHECK(testator_gaze->words().empty());
	CHECK(novice_gaze->words().empty());

	// A behest willed is beheld at once by both, as one and the same word.
	const std::shared_ptr<const Behest> behest = testator.will(shepherding, "fast");
	REQUIRE(testator_gaze->words().size() == 1);
	REQUIRE(novice_gaze->words().size() == 1);
	CHECK(testator_gaze->words()[0] == behest);
	CHECK(novice_gaze->words()[0] == behest);

	// The deed fulfilling it follows in the same stream, for both.
	const std::shared_ptr<const Deed> deed = novice.execute(*behest);
	REQUIRE(testator_gaze->words().size() == 2);
	CHECK(testator_gaze->words()[1] == deed);
	CHECK(novice_gaze->words()[1] == deed);

	// Asked by a side, the tie gives the very same living words.
	CHECK(shepherding.behests(testator).front() == behest);
	CHECK(obedience.deeds(novice).front() == deed);

}


TEST_CASE("the words of a tie are gone with the last gaze and are born anew from matter")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const auto& novice = static_cast<const Novice&>(world.welcome(DeviceToken::generate()));
	const auto& testator = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));

	novice.supplicate(testator);
	const Shepherding& shepherding = testator.accept(*testator.supplication(novice));

	testator.wake();
	testator.contemplate(shepherding);

	std::weak_ptr<const Behest> watched;
	id::Word behest_id{1};
	{
		const std::shared_ptr<const Behest> behest = testator.will(shepherding, "fast");
		watched = behest;
		behest_id = behest->id();
		novice.execute(*behest, Saying{"fasted"});
	}
	CHECK_FALSE(watched.expired());  // the testator's gaze holds it

	// The last gaze leaves: the living words are gone, their matter stays kept.
	testator.sleep();
	CHECK(watched.expired());

	// Turning anew, the tie births them again from matter, in their order.
	testator.wake();
	testator.contemplate(shepherding);
	const auto words = world.contemplation(testator.Soul::id())->words();
	REQUIRE(words.size() == 2);
	const auto behest = std::dynamic_pointer_cast<const Behest>(words[0]);
	const auto deed = std::dynamic_pointer_cast<const Deed>(words[1]);
	REQUIRE(behest);
	REQUIRE(deed);
	CHECK(behest->id() == behest_id);
	CHECK(deed->behest() == behest_id);
	CHECK(deed->saying().body() == "fasted");
}


TEST_CASE("tie is a place for distinct testator and novice")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const auto& testator = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));
	const auto& novice = static_cast<const Novice&>(world.welcome(DeviceToken::generate()));

	// A tie is born of the novice once the testator accepts his supplication; it is a place.
	static_assert(!std::is_constructible_v<Tie, matter::Tie>);
	novice.supplicate(testator);
	testator.accept(*testator.supplication(novice));

	const Obedience& tie = novice.obedience(testator);
	CHECK(id::Tie{tie.id()} == cosmos.spatiality().ties().front().id());
	CHECK(&Place::of(tie.id()) == static_cast<const Place*>(&tie));
	CHECK(&tie.testator() == &testator);
	CHECK(&tie.novice() == &novice);

	CHECK_THROWS_AS((matter::Tie{id::Tie{8}, testator.Soul::id(), testator.Soul::id()}),
					std::invalid_argument);
}


TEST_CASE("supplicate accept creates tie owned as obedience")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const Man& a = world.welcome(DeviceToken::generate());
	const Man& b = world.welcome(DeviceToken::generate());
	const auto& novice = static_cast<const Novice&>(a);
	const auto& testator = static_cast<const Testator&>(b);

	novice.supplicate(testator);
	CHECK(testator.supplications().size() == 1);
	const Supplication& ask = *testator.supplication(novice);
	CHECK(ask.suppliant().Soul::id() == a.Soul::id());
	CHECK(ask.addressee().Soul::id() == b.Soul::id());
	CHECK(cosmos.temporality().supplications(b.Soul::id()).size() == 1);

	// The supplication lives in Heaven, as a relation of the two souls.
	REQUIRE(world.supplications(b.Soul::id()).size() == 1);
	CHECK(world.supplications(b.Soul::id()).front().get() == &ask);
	CHECK(world.supplications(a.Soul::id()).empty());

	// Asked again while it awaits answer, the same one stays.
	CHECK_THROWS_AS(novice.supplicate(testator), std::logic_error);
	CHECK(testator.supplication(novice).get() == &ask);

	// Answered, Heaven lets it go; whoever still holds it keeps it a while.
	const std::shared_ptr<const Supplication> held = testator.supplication(novice);
	const std::weak_ptr<const Supplication> watched = held;
	const Shepherding& shepherding = testator.accept(ask);
	CHECK(testator.supplications().empty());
	CHECK(world.supplications(b.Soul::id()).empty());
	CHECK_FALSE(watched.expired());
	CHECK(&held->suppliant() == &novice);
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
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const Man& a = world.welcome(DeviceToken::generate());
	const Man& b = world.welcome(DeviceToken::generate());
	const auto& novice = static_cast<const Novice&>(a);
	const auto& testator = static_cast<const Testator&>(b);
	const auto& stranger = static_cast<const Testator&>(a);

	novice.supplicate(testator);
	const Supplication& ask = *testator.supplication(novice);
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
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const Man& a = world.welcome(DeviceToken::generate());
	const Man& b = world.welcome(DeviceToken::generate());
	CHECK(&world.man(a.name()) == &a);
	CHECK(&world.man(b.name()) == &b);
	CHECK_THROWS_AS(world.man(*SoulName::parse("zzzzzzzz")), std::invalid_argument);

	const auto& novice = static_cast<const Novice&>(a);
	const auto& testator = static_cast<const Testator&>(b);

	novice.supplicate(testator);
	testator.accept(*testator.supplication(novice));
	REQUIRE(cosmos.spatiality().ties().size() == 1);
	const Shepherding& shepherding = testator.shepherding(novice);
	const std::shared_ptr<const Behest> behest_held = testator.will(shepherding, "fast");
	const Behest& behest = *behest_held;
	CHECK(behest.saying().body() == "fast");
	CHECK(&behest.tie().testator() == &testator);
	CHECK(&behest.tie().novice() == &novice);

	CHECK_THROWS_AS(static_cast<const Testator&>(a).will(shepherding, "no"), std::logic_error);

	// The tie shows its behests to either side, through the one who asks.
	const Obedience& obedience = novice.obedience(testator);
	REQUIRE(obedience.behests(novice).size() == 1);
	REQUIRE(shepherding.behests(testator).size() == 1);
	CHECK(shepherding.behests(testator).front()->id() == behest.id());
	CHECK(obedience.deeds(novice).empty());
	CHECK_THROWS_AS(testator.execute(behest), std::logic_error);

	const std::shared_ptr<const Deed> deed = novice.execute(behest);
	CHECK(deed->behest() == behest.id());
	REQUIRE(shepherding.deeds(testator).size() == 1);
	CHECK(shepherding.deeds(testator).front()->id() == deed->id());

	// What is kept decides: a behest is fulfilled once, whatever snapshot one holds.
	CHECK_THROWS_AS(novice.execute(*obedience.behests(novice).front()), std::logic_error);
	CHECK_THROWS_AS(novice.execute(behest), std::logic_error);
	CHECK(cosmos.temporality().executions({behest.id()}).size() == 1);
}


TEST_CASE("a tie shows its behests only to its sides; behests are not letters")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const auto& novice = static_cast<const Novice&>(world.welcome(DeviceToken::generate()));
	const auto& testator = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));
	const auto& stranger = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));

	novice.supplicate(testator);
	testator.accept(*testator.supplication(novice));
	const std::shared_ptr<const Behest> behest_held = testator.will(testator.shepherding(novice), "fast");
	const Behest& behest = *behest_held;

	novice.wake();
	testator.wake();

	const Obedience& obedience = novice.obedience(testator);
	CHECK_THROWS_AS(obedience.behests(stranger), std::logic_error);
	CHECK(novice.obediences().size() == 1);
	CHECK(&novice.obediences().front().get() == &obedience);
	CHECK(stranger.obediences().empty());

	// A behest is placed in its tie, not in the abodes of its sides.
	CHECK(letters_seen_by(world.contemplation(testator.Soul::id())).empty());
	CHECK(letters_seen_by(world.contemplation(novice.Soul::id())).empty());

	// A letter is not a behest.
	static_cast<const Witness&>(novice).say("hello");
	REQUIRE(letters_seen_by(world.contemplation(novice.Soul::id())).size() == 1);
	REQUIRE(obedience.behests(novice).size() == 1);
	CHECK(obedience.behests(novice).front()->id() == behest.id());
}
