#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <optional>
#include "refusals/world.h"
#include "refusals/tie.h"
#include "refusals/testator.h"
#include "refusals/place.h"
#include "refusals/room.h"
#include "refusals/novice.h"
#include "refusals/training.h"
#include <doctest/doctest.h>

#include "domain_fakes.h"
#include "relations/friend.h"
#include "relations/neighbour.h"
#include "men/unborn.h"
#include "matter/training.h"
#include "values/exercise.h"
#include "places/room.h"

#include "horizons/creation.h"
#include "matter/abode.h"
#include "matter/supplication.h"
#include "relations/supplication.h"
#include "men/vessel.h"
#include "properties/birth.h"
#include "dimensions/temporality.h"
#include "dimensions/spatiality.h"
#include "horizons/life.h"
#include "relations/contemplation.h"
#include "places/obedience.h"
#include "places/shepherding.h"
#include "places/tie.h"
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
#include "words/training.h"
#include "refusals/refusal.h"
#include "words/word.h"
#include "properties/immanent.h"
#include "dimensions/eternity.h"
#include "words/letter.h"
#include "men/man.h"
#include "men/novice.h"
#include "men/testator.h"
#include "men/witness.h"
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
	const Man& man = born(world, token);
	const auto& witness = man;

	CHECK(man.Soul::id().value() > 0);
	CHECK(man.Vessel::id().value() > 0);
	CHECK(witness.abode().id().value() > 0);
	CHECK(witness.abode().dwells(man));

	// A man is born asleep: he contemplates nothing until he wakes.
	CHECK_FALSE(world.contemplation(man));
	CHECK(world.contemplating(man.abode()).empty());
	CHECK_THROWS_AS(write(world, man, "in my sleep"), std::logic_error);

	witness.wake();
	REQUIRE(world.contemplation(man));
	CHECK(&world.contemplation(man)->place() == &man.abode());
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

	const Man& man = born(world, test_token("abcd1234abcd1234abcd1234abcd1234"));
	CHECK(man.Soul::id() == id::Soul{42});
	CHECK(man.abode().dwells(man));
	CHECK(man.abode().id().value() > 0);
	CHECK(man.abode().id() != id::Place{man.Soul::id().value()});
}


TEST_CASE("welcome keeps existing name")
{
	InMemoryCosmos cosmos;
	seed_man(cosmos, id::Soul{7}, test_token("abcd1234abcd1234abcd1234abcd1234"), test_name("keptname"));
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	(void)born(world, test_token("abcd1234abcd1234abcd1234abcd1234"));

	CHECK(world.soul(id::Soul{7}).name() == test_name("keptname"));
}


TEST_CASE("each man has a distinct personal abode")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const Man& a = born(world, DeviceToken::generate());
	const Man& b = born(world, DeviceToken::generate());
	const auto& wa = a;
	const auto& wb = b;

	CHECK(wa.abode().id() != wb.abode().id());
	CHECK(&wa.abode() != &wb.abode());
	CHECK(wa.abode().dwells(a));
	CHECK_FALSE(wa.abode().dwells(b));
	CHECK(wb.abode().dwells(b));
	CHECK_FALSE(wb.abode().dwells(a));

	wa.wake();
	wb.wake();
	CHECK(&world.contemplation(a)->place() == &a.abode());
	CHECK(&world.contemplation(b)->place() == &b.abode());
	CHECK(world.contemplating(a.abode()).size() == 1);
	CHECK(world.contemplating(b.abode()).size() == 1);
	CHECK(&world.contemplation(a)->place() != &b.abode());
	CHECK(&world.contemplating(a.abode()).front().get() == static_cast<const Soul*>(&a));
	CHECK(&world.contemplating(b.abode()).front().get() == static_cast<const Soul*>(&b));
}


TEST_CASE("man say persists via temporality")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const Man& author = born(world, DeviceToken::generate());
	const auto& witness = author;
	witness.wake();
	witness.contemplate(cell_of(witness));
	write(world, author, "hello");

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

	const Man& man = born(world, test_token("feedfacefeedfacefeedfacefeedface"));
	const auto& witness = man;
	witness.wake();
	witness.contemplate(cell_of(witness));

	for (int i = 0; i < 5; ++i)
		write(world, man, "m" + std::to_string(i));

	const auto items = letters_seen_by(world.contemplation(witness));
	REQUIRE(items.size() == 5);
	CHECK(items.front()->saying().body() == "m0");
	CHECK(items.back()->saying().body() == "m4");
	CHECK(&items.front()->place() == &witness.abode());

	// Waking, a witness contemplates his own abode.
	const auto& stranger = born(world, DeviceToken::generate());
	stranger.wake();
	CHECK(&world.contemplation(stranger)->place() == &stranger.abode());
	CHECK(letters_seen_by(world.contemplation(stranger)).empty());

	// A contemplation is living: one gaze per soul, kept by Heaven, never copied.
	static_assert(!std::is_copy_constructible_v<Contemplation>);
	static_assert(!std::is_move_constructible_v<Contemplation>);
	CHECK(world.contemplation(stranger) == world.contemplation(stranger));
	CHECK(&world.contemplation(stranger)->who() == &stranger);
	CHECK(world.contemplating(stranger.abode()).size() == 1);

	// One cannot contemplate a place without dwelling in it: the gaze stays where it was.
	CHECK_THROWS_AS(stranger.contemplate(witness.abode()), std::logic_error);
	CHECK(&world.contemplation(stranger)->place() == &stranger.abode());
	CHECK(world.contemplating(witness.abode()).size() == 1);

	// Turning the gaze to an abode one dwells in ends the former gaze and begins a new one.
	const std::shared_ptr<const Contemplation> former = world.contemplation(stranger);
	stranger.contemplate(cell_of(stranger));
	CHECK(world.contemplation(stranger) != former);
	CHECK(world.contemplating(stranger.abode()).size() == 1);

	// An abode gives its recollection only to a contemplation of itself.
	CHECK_THROWS_AS(stranger.abode().recollection(*world.contemplation(witness)), std::logic_error);

	// A gaze that someone still holds does not fade under him when its soul sleeps.
	const std::shared_ptr<const Contemplation> held = world.contemplation(witness);
	witness.sleep();
	CHECK_FALSE(world.contemplation(witness));
	CHECK(world.contemplating(witness.abode()).empty());
	CHECK(letters_seen_by(held).size() == 5);
	CHECK_THROWS_AS(write(world, man, "asleep"), std::logic_error);

	// Sleeping twice is nothing; waking returns him to his own abode.
	witness.sleep();
	stranger.sleep();
	CHECK(world.contemplating(witness.abode()).empty());
	stranger.wake();
	CHECK(&world.contemplation(stranger)->place() == &stranger.abode());
}


TEST_CASE("letters of an abode live while it is contemplated, shared by every gaze")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const auto& host = born(world, DeviceToken::generate());
	// The forefather bore him standing at his gates: what was read for it is not counted.
	const std::size_t born_reads = cosmos.spatiality().placement_reads();

	// Asleep, nobody beholds the abode: nothing is read from the dimensions.
	CHECK(cosmos.spatiality().placement_reads() == born_reads);

	// The first gaze births the letters from what the dimensions keep.
	host.wake();
	host.contemplate(cell_of(host));
	CHECK(cosmos.spatiality().placement_reads() == born_reads + 1);
	CHECK(letters_seen_by(world.contemplation(host)).empty());

	// A letter said is born living at once and beheld, without reading the dimensions again.
	write(world, host, "first");
	write(world, host, "second");
	const auto seen = letters_seen_by(world.contemplation(host));
	REQUIRE(seen.size() == 2);
	CHECK(seen[0]->saying().body() == "first");
	CHECK(seen[1]->saying().body() == "second");
	CHECK(&seen[0]->author() == static_cast<const Soul*>(&host));
	CHECK(cosmos.spatiality().placement_reads() == born_reads + 1);

	// A new gaze upon the same abode shares the very same letters: nothing is read anew.
	const std::shared_ptr<const Contemplation> former = world.contemplation(host);
	host.contemplate(cell_of(host));
	const std::shared_ptr<const Contemplation> current = world.contemplation(host);
	REQUIRE(current != former);
	CHECK(letters_seen_by(current)[0] == seen[0]);
	CHECK(letters_seen_by(current)[1] == seen[1]);
	CHECK(letters_seen_by(former)[0] == seen[0]);
	CHECK(cosmos.spatiality().placement_reads() == born_reads + 1);

	// Every gaze still held upon the abode shares its one recollection, and
	// beholds what is said next.
	write(world, host, "third");
	CHECK(letters_seen_by(current).size() == 3);
	CHECK(letters_seen_by(former).size() == 3);
	CHECK(letters_seen_by(former)[2] == letters_seen_by(current)[2]);

	// What is said in another abode enters only that abode's recollection.
	const auto& other = born(world, DeviceToken::generate());
	other.wake();
	other.contemplate(cell_of(other));
	write(world, other, "elsewhere");
	CHECK(letters_seen_by(current).size() == 3);
	CHECK(letters_seen_by(world.contemplation(other)).size() == 1);
}


TEST_CASE("the words of a place live together in its recollection, all or none")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const auto& host = born(world, DeviceToken::generate());

	host.wake();
	host.contemplate(cell_of(host));
	write(world, host, "first");
	write(world, host, "second");

	// While any gaze holds the recollection, every word in it lives, even one
	// said after that gaze was let go by Heaven.
	std::shared_ptr<const Contemplation> old_gaze = world.contemplation(host);
	host.contemplate(cell_of(host));
	write(world, host, "third");
	const std::weak_ptr<const Letter> first = letters_seen_by(old_gaze).front();
	const std::weak_ptr<const Letter> third = letters_seen_by(world.contemplation(host)).back();
	host.sleep();
	CHECK_FALSE(third.expired());
	REQUIRE(letters_seen_by(old_gaze).size() == 3);

	// The last gaze is let go: all the words end together, their memory stays kept.
	old_gaze.reset();
	CHECK(first.expired());
	CHECK(third.expired());

	// Waking, he recollects all three anew from the dimensions.
	const std::size_t reads = cosmos.spatiality().placement_reads();
	host.wake();
	host.contemplate(cell_of(host));
	const auto seen = letters_seen_by(world.contemplation(host));
	REQUIRE(seen.size() == 3);
	CHECK(seen[0]->saying().body() == "first");
	CHECK(seen[2]->saying().body() == "third");
	CHECK(cosmos.spatiality().placement_reads() == reads + 1);
}


TEST_CASE("letters of an abode are gone with the last gaze and are born anew from matter")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const auto& host = born(world, DeviceToken::generate());
	// The forefather bore him standing at his gates: what was read for it is not counted.
	const std::size_t born_reads = cosmos.spatiality().placement_reads();

	host.wake();
	host.contemplate(cell_of(host));
	write(world, host, "kept");
	std::weak_ptr<const Letter> watched = letters_seen_by(world.contemplation(host)).front();
	REQUIRE(watched.lock());
	CHECK(cosmos.spatiality().placement_reads() == born_reads + 1);

	// While anyone still holds a gaze, its letters live on though the man sleeps.
	std::shared_ptr<const Contemplation> held = world.contemplation(host);
	host.sleep();
	CHECK_FALSE(watched.expired());

	// The last gaze is let go: the living letter is gone, its matter stays kept.
	held.reset();
	CHECK(watched.expired());
	CHECK(cosmos.spatiality().placements(host.abode().id(), 10).size() == 1);
	const std::size_t reads = cosmos.spatiality().placement_reads();

	// A new gaze births the letters anew from the dimensions.
	host.wake();
	host.contemplate(cell_of(host));
	CHECK(cosmos.spatiality().placement_reads() == reads + 1);
	const auto reborn = letters_seen_by(world.contemplation(host));
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
	static_assert(!std::is_constructible_v<Birth<Place>, const Place&>);

	// And without the key nothing living is brought forth from its matter.
	static_assert(!std::is_constructible_v<Testator, matter::Man>);
	static_assert(!std::is_constructible_v<Vessel, matter::Vessel>);
	static_assert(!std::is_constructible_v<Abode, matter::Abode>);
	static_assert(!std::is_constructible_v<Tie, matter::Tie>);
	static_assert(!std::is_copy_constructible_v<Tie>);
	static_assert(!std::is_move_constructible_v<Tie>);
	static_assert(!std::is_copy_assignable_v<Tie>);
	static_assert(!std::is_move_assignable_v<Tie>);
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
	const Man& man = born(creation.world(), DeviceToken::generate());
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
		soul_id = born(creation.world(), token).Soul::id();
		CHECK_THROWS_AS(cosmos.life().create(), std::logic_error);
		CHECK(&cosmos.life().creation() == &creation);
	}

	InMemoryCosmos cosmos(matter);
	CHECK_THROWS_AS(cosmos.life().creation(), std::logic_error);
	Creation& creation = cosmos.life().create();
	CHECK(creation.world().knows(*soul_id));
	CHECK(born(creation.world(), token).Soul::id() == *soul_id);
}


TEST_CASE("a word is immanent to Life and is neither copied nor moved in any role")
{
	static_assert(std::is_base_of_v<Immanent<Life>, Word>);
	static_assert(std::is_base_of_v<Immanent<Life>, Recollection>);
	static_assert(!std::is_copy_constructible_v<Recollection>);
	static_assert(!std::is_move_constructible_v<Recollection>);
	static_assert(!std::is_constructible_v<Recollection, const Place&, std::vector<std::shared_ptr<const Word>>>);
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

	const auto& novice = born(world, DeviceToken::generate());
	const auto& testator = born(world, DeviceToken::generate());
	const auto& stranger = born(world, DeviceToken::generate());

	admit_at_gates(world, novice, testator);
	admit_at_gates(world, testator, novice);
	novice.supplicate(testator);
	const Shepherding& shepherding = testator.accept(*testator.supplication(novice));
	testator.wake();
	testator.contemplate(room_in(testator, shepherding));
	const std::shared_ptr<const Behest> willed_held = will_here(world, testator, "fast");
	const Behest& willed = *willed_held;

	novice.wake();
	testator.wake();
	stranger.wake();

	// Both sides dwell in the tie and see it through their rooms; a stranger has no such room.
	CHECK(shepherding.dwells(novice));
	CHECK(shepherding.dwells(testator));
	CHECK_FALSE(shepherding.dwells(stranger));
	CHECK_FALSE(stranger.abode().cell(shepherding));

	// Turning to the tie, a side beholds its behests as words.
	testator.contemplate(room_in(testator, shepherding));
	const std::shared_ptr<const Contemplation> gaze = world.contemplation(testator);
	CHECK(&gaze->place().source() == static_cast<const Place*>(&shepherding));
	REQUIRE(gaze->words().size() == 1);
	const auto behest = std::dynamic_pointer_cast<const Behest>(gaze->words().front());
	REQUIRE(behest);
	CHECK(behest->id() == willed.id());
	CHECK(world.contemplating(shepherding).size() == 1);

	// What he writes in the tie, he wills.
	CHECK(will_here(world, testator, "hello"));

	// Turning to his cell, he writes a letter there.
	testator.contemplate(cell_of(testator));
	CHECK(world.contemplating(shepherding).empty());
	CHECK(std::dynamic_pointer_cast<const Letter>(write(world, testator, "hello")));
}


TEST_CASE("the words of a tie live while it is contemplated, one for both sides")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const auto& novice = born(world, DeviceToken::generate());
	const auto& testator = born(world, DeviceToken::generate());

	admit_at_gates(world, novice, testator);
	admit_at_gates(world, testator, novice);
	novice.supplicate(testator);
	const Shepherding& shepherding = testator.accept(*testator.supplication(novice));
	const Obedience& obedience = novice.obedience(testator);

	// Both sides turn to the tie: the testator's gaze births its words, the novice's shares them.
	novice.wake();
	testator.wake();
	testator.contemplate(room_in(testator, shepherding));
	novice.contemplate(room_in(novice, obedience));
	const std::shared_ptr<const Contemplation> testator_gaze = world.contemplation(testator);
	const std::shared_ptr<const Contemplation> novice_gaze = world.contemplation(novice);
	CHECK(testator_gaze->words().empty());
	CHECK(novice_gaze->words().empty());

	// A behest willed is beheld at once by both, as one and the same word.
	const std::shared_ptr<const Behest> behest = will_here(world, testator, "fast");
	REQUIRE(testator_gaze->words().size() == 1);
	REQUIRE(novice_gaze->words().size() == 1);
	CHECK(testator_gaze->words()[0] == behest);
	CHECK(novice_gaze->words()[0] == behest);

	// The deed fulfilling it follows in the same stream, for both.
	const std::shared_ptr<const Deed> deed = execute(world, novice, *behest);
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

	const auto& novice = born(world, DeviceToken::generate());
	const auto& testator = born(world, DeviceToken::generate());

	admit_at_gates(world, novice, testator);
	admit_at_gates(world, testator, novice);
	novice.supplicate(testator);
	const Shepherding& shepherding = testator.accept(*testator.supplication(novice));

	testator.wake();
	testator.contemplate(room_in(testator, shepherding));

	std::weak_ptr<const Behest> watched;
	id::Word behest_id{1};
	{
		const std::shared_ptr<const Behest> behest = will_here(world, testator, "fast");
		watched = behest;
		behest_id = behest->id();
		novice.wake();
		novice.contemplate(room_in(novice, novice.obedience(testator)));
		execute(world, novice, *behest, Saying{"fasted"});
		novice.sleep();
	}
	CHECK_FALSE(watched.expired());  // the testator's gaze holds it

	// The last gaze leaves: the living words are gone, their matter stays kept.
	testator.sleep();
	CHECK(watched.expired());

	// Turning anew, the tie births them again from matter, in their order.
	testator.wake();
	testator.contemplate(room_in(testator, shepherding));
	const auto words = world.contemplation(testator)->words();
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

	const auto& testator = born(world, DeviceToken::generate());
	const auto& novice = born(world, DeviceToken::generate());

	// A tie is born of the novice once the testator accepts his supplication; it is a place.
	static_assert(!std::is_constructible_v<Tie, matter::Tie>);
	admit_at_gates(world, novice, testator);
	admit_at_gates(world, testator, novice);
	novice.supplicate(testator);
	testator.accept(*testator.supplication(novice));

	const Obedience& tie = novice.obedience(testator);
	CHECK(tie.id() == cosmos.spatiality().ties().front().id());
	CHECK(&Place::of(tie.id()) == static_cast<const Place*>(&tie));
	CHECK(&tie.testator() == &testator);
	CHECK(&tie.novice() == &novice);

	CHECK_THROWS_AS((matter::Tie{id::Place{8}, testator.Soul::id(), testator.Soul::id()}),
					std::invalid_argument);
}


TEST_CASE("supplicate accept creates tie owned as obedience")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const Man& a = born(world, DeviceToken::generate());
	const Man& b = born(world, DeviceToken::generate());
	const auto& novice = a;
	const auto& testator = b;

	admit_at_gates(world, novice, testator);
	admit_at_gates(world, testator, novice);
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
	CHECK(shepherding.id() == cosmos.spatiality().ties().front().id());
	CHECK(&testator.shepherding(novice) == &shepherding);
	CHECK_THROWS_AS(a.shepherding(novice), std::invalid_argument);
	const Obedience& obedience = novice.obedience(testator);
	CHECK(&obedience.testator() == &testator);
	CHECK(&obedience.novice() == &novice);
	CHECK(obedience.id() == shepherding.id());
	CHECK(obedience.id().value() != a.Soul::id().value());
	CHECK(obedience.id().value() != b.Soul::id().value());
	CHECK_THROWS_AS(testator.obedience(a), std::invalid_argument);

	// The supplication is answered in time: accepted.
	REQUIRE(cosmos.shared().answers.size() == 1);
	CHECK(cosmos.shared().answers.front().form() == matter::Answer::Form::Accepted);
	CHECK(cosmos.temporality().supplications(b.Soul::id()).empty());

	// The pair is tied: the testator does not hear the novice again, and no new bond is kept.
	CHECK_THROWS_AS(novice.supplicate(testator), std::logic_error);
	CHECK(cosmos.temporality().supplications(b.Soul::id()).empty());
	CHECK_THROWS_AS(cosmos.spatiality().bind(b.Soul::id(), a.Soul::id()), std::logic_error);
	CHECK(cosmos.spatiality().ties().size() == 1);
}


TEST_CASE("reject closes pending supplication; wrong party cannot accept")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const Man& a = born(world, DeviceToken::generate());
	const Man& b = born(world, DeviceToken::generate());
	const auto& novice = a;
	const auto& testator = b;
	const auto& stranger = a;

	admit_at_gates(world, novice, testator);
	admit_at_gates(world, testator, novice);
	novice.supplicate(testator);
	const Supplication& ask = *testator.supplication(novice);
	CHECK_THROWS_AS(stranger.accept(ask), std::logic_error);

	testator.reject(ask);
	CHECK(testator.supplications().empty());
	CHECK(cosmos.temporality().supplications(b.Soul::id()).empty());
	CHECK_THROWS_AS(testator.supplication(novice), Refusal);
	REQUIRE(cosmos.shared().answers.size() == 1);
	CHECK(cosmos.shared().answers.front().form() == matter::Answer::Form::Rejected);
	CHECK_THROWS_AS(cosmos.temporality().answer(a.Soul::id(), b.Soul::id(), matter::Answer::Form::Rejected),
					std::invalid_argument);
	CHECK(cosmos.spatiality().ties().empty());

	// A rejected suppliant may ask again; a second supplication while one awaits is refused.
	novice.supplicate(testator);
	CHECK(cosmos.temporality().supplications(b.Soul::id()).size() == 1);
	CHECK_THROWS_AS(novice.supplicate(testator), std::logic_error);
	CHECK_THROWS_AS(novice.supplicate(a), Refusal);
}


TEST_CASE("will and execute within obedience")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const Man& a = born(world, DeviceToken::generate());
	const Man& b = born(world, DeviceToken::generate());
	CHECK(&world.man(a.name()) == &a);
	CHECK(&world.man(b.name()) == &b);
	CHECK_THROWS_AS(world.man(*SoulName::parse("zzzzzzzz")), std::invalid_argument);

	const auto& novice = a;
	const auto& testator = b;

	admit_at_gates(world, novice, testator);
	admit_at_gates(world, testator, novice);
	novice.supplicate(testator);
	testator.accept(*testator.supplication(novice));
	REQUIRE(cosmos.spatiality().ties().size() == 1);
	const Shepherding& shepherding = testator.shepherding(novice);
	testator.wake();
	testator.contemplate(room_in(testator, shepherding));
	const std::shared_ptr<const Behest> behest_held = will_here(world, testator, "fast");
	const Behest& behest = *behest_held;
	CHECK(behest.saying().body() == "fast");
	CHECK(&behest.tie().testator() == &testator);
	CHECK(&behest.tie().novice() == &novice);

	// The tie shows its behests to either side, through the one who asks.
	const Obedience& obedience = novice.obedience(testator);
	REQUIRE(obedience.behests(novice).size() == 1);
	REQUIRE(shepherding.behests(testator).size() == 1);
	CHECK(shepherding.behests(testator).front()->id() == behest.id());
	CHECK(obedience.deeds(novice).empty());
	CHECK_THROWS_AS(execute(world, testator, behest), std::logic_error);

	// One fulfils a behest only while contemplating its tie.
	CHECK_THROWS_AS(execute(world, novice, behest), std::logic_error);
	novice.wake();
	novice.contemplate(room_in(novice, obedience));
	const std::shared_ptr<const Deed> deed = execute(world, novice, behest);
	CHECK(deed->behest() == behest.id());
	REQUIRE(shepherding.deeds(testator).size() == 1);
	CHECK(shepherding.deeds(testator).front()->id() == deed->id());

	// What is kept decides: a behest is fulfilled once, whatever snapshot one holds.
	CHECK_THROWS_AS(execute(world, novice, *obedience.behests(novice).front()), std::logic_error);
	CHECK_THROWS_AS(execute(world, novice, behest), std::logic_error);
	CHECK(cosmos.temporality().executions({behest.id()}).size() == 1);
}


TEST_CASE("a tie shows its behests only to its sides; behests are not letters")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const auto& novice = born(world, DeviceToken::generate());
	const auto& testator = born(world, DeviceToken::generate());
	const auto& stranger = born(world, DeviceToken::generate());

	admit_at_gates(world, novice, testator);
	admit_at_gates(world, testator, novice);
	novice.supplicate(testator);
	testator.accept(*testator.supplication(novice));
	testator.wake();
	CHECK_THROWS_AS(will_here(world, testator, "fast"), std::logic_error);
	testator.contemplate(room_in(testator, testator.shepherding(novice)));
	const std::shared_ptr<const Behest> behest_held = will_here(world, testator, "fast");
	const Behest& behest = *behest_held;

	novice.wake();
	testator.wake();

	const Obedience& obedience = novice.obedience(testator);
	CHECK_THROWS_AS(obedience.behests(stranger), std::logic_error);
	CHECK(novice.obediences().size() == 1);
	CHECK(&novice.obediences().front().get() == &obedience);
	CHECK(stranger.obediences().empty());

	// A behest is placed in its tie, not in the abodes of its sides.
	CHECK(letters_seen_by(world.contemplation(testator)).empty());
	CHECK(letters_seen_by(world.contemplation(novice)).empty());

	// A letter is not a behest.
	novice.contemplate(cell_of(novice));
	write(world, novice, "hello");
	REQUIRE(letters_seen_by(world.contemplation(novice)).size() == 1);
	REQUIRE(obedience.behests(novice).size() == 1);
	CHECK(obedience.behests(novice).front()->id() == behest.id());
}


TEST_CASE("the host admits a man as an acquaintance and regards him anew")
{
	InMemoryCosmos cosmos;
	Creation& creation = cosmos.life().create();
	World& world = creation.world();

	const auto& host = born(world, DeviceToken::generate());
	const auto& man = born(world, DeviceToken::generate());

	// The host dwells in his abode, but is not a dweller of it; a stranger does not dwell.
	CHECK(&host.abode().host() == static_cast<const Man*>(&host));
	CHECK(host.abode().dwells(host));
	CHECK_FALSE(host.abode().dweller(host));
	CHECK_FALSE(host.abode().dwells(man));
	CHECK_THROWS_AS(regard(world, host, man, matter::Dweller::Kind::Friend), std::logic_error);
	CHECK_THROWS_AS(admit(world, host, host), std::logic_error);

	// Admitted, he is an acquaintance; regarded anew, a friend. Matter keeps it.
	admit_at_gates(world, host, man);
	CHECK(host.abode().dwells(man));
	REQUIRE(host.abode().dweller(man));
	CHECK_FALSE(std::dynamic_pointer_cast<const Neighbour>(host.abode().dweller(man)));
	CHECK_THROWS_AS(admit(world, host, man), std::logic_error);
	regard_in_upper_room(world, host, man, matter::Dweller::Kind::Friend);
	CHECK(std::dynamic_pointer_cast<const Friend>(host.abode().dweller(man)));
	std::vector<matter::Dweller> kept;
	for (const matter::Dweller& dweller : cosmos.spatiality().dwellers()) {
		if (dweller.abode() == host.abode().id() && dweller.soul() == man.Soul::id())
			kept.push_back(dweller);
	}
	REQUIRE(kept.size() == 1);
	CHECK(kept.front().kind() == matter::Dweller::Kind::Friend);

	// Admission is not mutual: the host does not dwell in the man's abode.
	CHECK_FALSE(man.abode().dwells(host));

	// A dweller may turn his gaze to the abode, but one writes only in a cell.
	man.wake();
	man.contemplate(host.abode());
	CHECK_THROWS_AS(write(world, man, "not mine"), StandInCell);

	// Entering the host's cell, the friend writes there as the host: the letter is his.
	man.contemplate(cell_of(host));
	CHECK(writes_here(world, man));
	const auto letter = std::dynamic_pointer_cast<const Letter>(write(world, man, "for the host"));
	REQUIRE(letter);
	CHECK(&letter->author() == static_cast<const Soul*>(&host));
	CHECK(&letter->place() == &host.abode());
}


TEST_CASE("dwellers outlive Life in matter and are recalled on awakening")
{
	InMemoryShared store;
	const DeviceToken host_token = DeviceToken::generate();
	const DeviceToken man_token = DeviceToken::generate();
	{
		InMemoryCosmos cosmos(store);
		World& world = cosmos.life().create().world();
		const Man& host = born(world, host_token);
		const Man& man = born(world, man_token);
		admit_at_gates(world, host, man);
		regard_in_upper_room(world, host, man, matter::Dweller::Kind::Neighbour);
	}

	InMemoryCosmos cosmos(store);
	World& world = cosmos.life().create().world();
	const Man& host = born(world, host_token);
	const Man& man = born(world, man_token);
	CHECK(std::dynamic_pointer_cast<const Neighbour>(host.abode().dweller(man)));
	CHECK_FALSE(std::dynamic_pointer_cast<const Friend>(host.abode().dweller(man)));
	CHECK_FALSE(man.abode().dwells(host));
}


TEST_CASE("a dweller enters the rooms his kind opens: a friend all, a neighbour the outer part")
{
	static_assert(std::is_base_of_v<Acquaintance, Neighbour>);
	static_assert(std::is_base_of_v<Neighbour, Friend>);
	static_assert(!std::is_constructible_v<Acquaintance, const Man&>);
	static_assert(!std::is_copy_constructible_v<Acquaintance>);

	InMemoryCosmos cosmos;
	World& world = cosmos.life().create().world();

	const auto& host = born(world, DeviceToken::generate());
	const auto& man = born(world, DeviceToken::generate());
	const Room& cell = *host.abode().cell(host.abode());

	host.wake();
	host.contemplate(cell_of(host));
	write(world, host, "first");
	admit_at_gates(world, host, man);
	// Looking at the abode itself, a dweller sees no words: they are seen in its rooms.
	man.wake();
	man.contemplate(host.abode());
	CHECK(world.contemplation(man)->words().empty());

	// The cell stands in the inner part: an acquaintance and a neighbour do not enter it.
	CHECK_FALSE(cell.dwells(man));
	CHECK_THROWS_AS(man.contemplate(cell), std::logic_error);
	regard_in_upper_room(world, host, man, matter::Dweller::Kind::Neighbour);
	CHECK_FALSE(cell.dwells(man));
	CHECK_THROWS_AS(man.contemplate(cell), std::logic_error);

	// A friend enters all rooms, and sees the words.
	regard_in_upper_room(world, host, man, matter::Dweller::Kind::Friend);
	CHECK(cell.dwells(man));
	man.contemplate(cell);
	const std::shared_ptr<const Contemplation> gaze = world.contemplation(man);
	CHECK(letters_seen_by(gaze).size() == 1);

	// Rooms are arranged only in the upper room.
	CHECK_THROWS_AS(arrange(world, host, cell, matter::Room::Part::Outer), std::logic_error);
	CHECK(cell.part() == matter::Room::Part::Inner);

	// Set in the outer part, the cell is entered by a neighbour too, but never by an acquaintance.
	arrange_in_upper_room(world, host, cell, matter::Room::Part::Outer);
	CHECK(cell.part() == matter::Room::Part::Outer);
	regard_in_upper_room(world, host, man, matter::Dweller::Kind::Neighbour);
	CHECK(cell.dwells(man));
	CHECK(letters_seen_by(gaze).size() == 1);
	regard_in_upper_room(world, host, man, matter::Dweller::Kind::Acquaintance);
	CHECK_FALSE(cell.dwells(man));
	CHECK(gaze->words().empty());

	// Only the host arranges, and only rooms of his own abode.
	CHECK_THROWS_AS(arrange(world, man, cell, matter::Room::Part::Inner), std::logic_error);
}


TEST_CASE("a tie is reflected in the abode of each side: Ведение and Послушание")
{
	InMemoryShared store;
	const DeviceToken novice_token = DeviceToken::generate();
	const DeviceToken testator_token = DeviceToken::generate();
	std::string novice_name;
	std::string testator_name;
	{
		InMemoryCosmos cosmos(store);
		World& world = cosmos.life().create().world();
		const auto& novice = born(world, novice_token);
		const auto& testator = born(world, testator_token);
		novice_name = std::string{novice.name().text()};
		testator_name = std::string{testator.name().text()};
		admit_at_gates(world, novice, testator);
		admit_at_gates(world, testator, novice);
		novice.supplicate(testator);
		const Shepherding& shepherding = testator.accept(*testator.supplication(novice));

		const Cell* leading = testator.abode().cell(shepherding);
		const Cell* obeying = novice.abode().cell(shepherding);
		REQUIRE(leading);
		REQUIRE(obeying);
		CHECK(leading->name() == "Ведение — " + novice_name);
		CHECK(obeying->name() == "Послушание — " + testator_name);
		CHECK(leading->part() == matter::Room::Part::Inner);
		CHECK(testator.abode().rooms().size() == 4);  // the cell, the gates, the upper room, Ведение
	}

	// Awakening anew, the tie rooms are recalled with the tie.
	InMemoryCosmos cosmos(store);
	World& world = cosmos.life().create().world();
	const auto& novice = born(world, novice_token);
	const auto& testator = born(world, testator_token);
	CHECK(testator.abode().room("Ведение — " + novice_name));
	CHECK(novice.abode().room("Послушание — " + testator_name));
	CHECK(store.rooms.size() == 11);  // three standard rooms in each of three abodes (the forefather's too), two tie rooms
}


TEST_CASE("a tie bound before rooms were is given its rooms on awakening")
{
	InMemoryShared store;
	const DeviceToken novice_token = DeviceToken::generate();
	const DeviceToken testator_token = DeviceToken::generate();
	{
		InMemoryCosmos cosmos(store);
		World& world = cosmos.life().create().world();
		const auto& novice = born(world, novice_token);
		const auto& testator = born(world, testator_token);
		admit_at_gates(world, novice, testator);
		admit_at_gates(world, testator, novice);
		novice.supplicate(testator);
		testator.accept(*testator.supplication(novice));
	}
	// As kept before tie rooms were: only the standard rooms remain.
	std::erase_if(store.rooms, [](const matter::Room& room) { return room.reflects() != room.abode(); });
	REQUIRE(store.rooms.size() == 9);

	InMemoryCosmos cosmos(store);
	World& world = cosmos.life().create().world();
	const auto& novice = born(world, novice_token);
	CHECK(novice.abode().rooms().size() == 4);
	CHECK(store.rooms.size() == 11);
}


TEST_CASE("every abode has its cell, a room reflecting the abode itself, kept once")
{
	InMemoryShared store;
	const DeviceToken token = DeviceToken::generate();
	{
		InMemoryCosmos cosmos(store);
		World& world = cosmos.life().create().world();
		const Man& man = born(world, token);

		REQUIRE(man.abode().rooms().size() == 3);
		const Room& cell = man.abode().rooms().front();
		CHECK(cell.name() == "Слово");
		CHECK(&cell.reflects() == static_cast<const Place*>(&man.abode()));
		CHECK(&cell.abode() == &man.abode());
		CHECK(cell.part() == matter::Room::Part::Inner);
		CHECK(man.abode().room("Слово") == &cell);
		CHECK(man.abode().cell(man.abode()) == &cell);
		CHECK(cell.dwells(man));
		CHECK_THROWS_AS(cosmos.spatiality().furnish(man.abode().id(), man.abode().id(), matter::Room::Aspect::Words),
						std::logic_error);
	}

	// Awakening anew, the cell is recalled, not furnished again.
	InMemoryCosmos cosmos(store);
	const Man& man = born(cosmos.life().create().world(), token);
	REQUIRE(man.abode().rooms().size() == 3);
	CHECK(store.rooms.size() == 6);  // and the forefather's three
	const id::Place cell = man.abode().rooms().front().get().id();
	CHECK(std::count_if(store.rooms.begin(), store.rooms.end(), [&](const matter::Room& room) {
		return room.id() == cell;
	}) == 1);
}


TEST_CASE("one writes only in one's cell; the abode itself shows no words")
{
	InMemoryCosmos cosmos;
	World& world = cosmos.life().create().world();
	const auto& host = born(world, DeviceToken::generate());

	// Woken, he looks at his abode itself: there he does not write, and sees no words.
	host.wake();
	CHECK(&world.contemplation(host)->place() == static_cast<const Place*>(&host.abode()));
	CHECK_THROWS_AS(write(world, host, "at the threshold"), std::logic_error);

	host.contemplate(cell_of(host));
	write(world, host, "in the cell");
	CHECK(letters_seen_by(world.contemplation(host)).size() == 1);

	host.contemplate(host.abode());
	CHECK(world.contemplation(host)->words().empty());

	// Through his cell he contemplates the words of his abode.
	host.contemplate(cell_of(host));
	CHECK(host.contemplates(host.abode()));
	CHECK(world.contemplating(host.abode()).size() == 1);
}


TEST_CASE("the abode shows its host what awaits his attention and what he has set for others")
{
	InMemoryCosmos cosmos;
	World& world = cosmos.life().create().world();
	const auto& novice = born(world, DeviceToken::generate());
	const auto& testator = born(world, DeviceToken::generate());
	admit_at_gates(world, novice, testator);
	admit_at_gates(world, testator, novice);
	novice.supplicate(testator);
	const Shepherding& shepherding = testator.accept(*testator.supplication(novice));
	CHECK(novice.abode().owed().empty());

	// The testator wills through his room Ведение, as through the tie itself.
	testator.wake();
	testator.contemplate(*testator.abode().cell(shepherding));
	const std::shared_ptr<const Behest> fast = will_here(world, testator, "fast");
	const std::shared_ptr<const Behest> pray = will_here(world, testator, "pray");

	REQUIRE(novice.abode().owed().size() == 2);
	CHECK(novice.abode().owed()[0]->id() == fast->id());
	// The testator does not owe what he has set: it is set for the novice.
	CHECK(testator.abode().owed().empty());
	REQUIRE(testator.abode().awaited().size() == 2);
	CHECK(novice.abode().awaited().empty());

	// Fulfilled through the room Послушание, a behest no longer awaits.
	novice.wake();
	novice.contemplate(*novice.abode().cell(shepherding));
	execute(world, novice, *fast);
	REQUIRE(novice.abode().owed().size() == 1);
	CHECK(novice.abode().owed()[0]->id() == pray->id());
	CHECK(testator.abode().awaited().size() == 1);
}


TEST_CASE("one comes into another's abode through its gates, let in by its host standing there")
{
	InMemoryCosmos cosmos;
	World& world = cosmos.life().create().world();
	const auto& host = born(world, DeviceToken::generate());
	const auto& man = born(world, DeviceToken::generate());
	const auto& passer = born(world, DeviceToken::generate());
	const Gates& gates = host.abode().gates();
	CHECK(gates.name() == "Врата");

	// A stranger may not look at the abode itself, but anyone may stand at its gates.
	man.wake();
	CHECK_THROWS_AS(man.contemplate(host.abode()), std::logic_error);
	man.contemplate(gates);
	CHECK(world.contemplation(man)->words().empty());

	// The host is elsewhere: the gates are shut, and he lets no one in.
	host.wake();
	CHECK_FALSE(gates.open());
	CHECK_THROWS_AS(admit(world, host, man), std::logic_error);

	// Standing in his gates, he opens them and lets in the one who stands there.
	host.contemplate(gates);
	CHECK(gates.open());
	admit(world, host, man);
	CHECK(host.abode().dweller(man));

	// One who has left the gates can no longer be let in.
	passer.wake();
	passer.contemplate(gates);
	passer.contemplate(passer.abode());
	CHECK_THROWS_AS(admit(world, host, passer), std::logic_error);
	CHECK_FALSE(host.abode().dweller(passer));
}


TEST_CASE("the host regards his dwellers anew only in his upper room; it shows no words")
{
	InMemoryCosmos cosmos;
	World& world = cosmos.life().create().world();
	const auto& host = born(world, DeviceToken::generate());
	const auto& man = born(world, DeviceToken::generate());
	const UpperRoom& upper_room = host.abode().upper_room();
	CHECK(upper_room.name() == "Горница");
	CHECK(upper_room.part() == matter::Room::Part::Inner);

	host.wake();
	man.wake();
	admit_at_gates(world, host, man);

	// Elsewhere he may not regard him anew; in his upper room he may.
	CHECK_THROWS_AS(regard(world, host, man, matter::Dweller::Kind::Friend), std::logic_error);
	host.contemplate(upper_room);
	regard(world, host, man, matter::Dweller::Kind::Friend);
	CHECK(std::dynamic_pointer_cast<const Friend>(host.abode().dweller(man)));

	// The upper room is a room like any: a friend enters it, and sees no words there.
	CHECK(upper_room.dwells(man));
	man.contemplate(upper_room);
	CHECK(world.contemplation(man)->words().empty());
}


TEST_CASE("whoever stands in an upper room regards and arranges there as its host, but does not regard himself")
{
	InMemoryCosmos cosmos;
	World& world = cosmos.life().create().world();
	const auto& host = born(world, DeviceToken::generate());
	const auto& friend_ = born(world, DeviceToken::generate());
	const auto& other = born(world, DeviceToken::generate());
	const UpperRoom& upper_room = host.abode().upper_room();
	admit_at_gates(world, host, friend_);
	admit_at_gates(world, host, other);
	regard_in_upper_room(world, host, friend_, matter::Dweller::Kind::Friend);

	// The friend, in the host's upper room, regards another anew and sets a room in a part.
	friend_.wake();
	friend_.contemplate(upper_room);
	regard(world, friend_, other, matter::Dweller::Kind::Neighbour);
	CHECK(std::dynamic_pointer_cast<const Neighbour>(host.abode().dweller(other)));
	arrange(world, friend_, cell_of(host), matter::Room::Part::Outer);
	CHECK(cell_of(host).part() == matter::Room::Part::Outer);

	// Himself he does not regard.
	CHECK_THROWS_AS(regard(world, friend_, friend_, matter::Dweller::Kind::Acquaintance), RegardNotOneself);

	// Nor does a neighbour raise himself, though the upper room stands in the outer part.
	arrange(world, friend_, upper_room, matter::Room::Part::Outer);
	other.wake();
	other.contemplate(upper_room);
	CHECK_THROWS_AS(regard(world, other, other, matter::Dweller::Kind::Friend), RegardNotOneself);
	CHECK_FALSE(std::dynamic_pointer_cast<const Friend>(host.abode().dweller(other)));
}


TEST_CASE("the first man is begotten at once; a body coming after awaits its birth")
{
	InMemoryShared store;
	const DeviceToken adam_token = DeviceToken::generate();
	const DeviceToken seth_token = DeviceToken::generate();
	id::Vessel mark{1};
	{
		InMemoryCosmos cosmos(store);
		World& world = cosmos.life().create().world();

		const Vessel& adam = world.welcome(adam_token);
		CHECK(dynamic_cast<const Man*>(&adam));
		CHECK(world.unborn().empty());

		const Vessel& seth = world.welcome(seth_token);
		REQUIRE(dynamic_cast<const Unborn*>(&seth));
		mark = seth.id();
		REQUIRE(world.unborn().size() == 1);
		CHECK(&world.unborn().front().get() == &seth);
		CHECK(&world.unborn(mark) == &seth);
		CHECK(&world.welcome(seth_token) == &seth);
		CHECK(store.souls.size() == 1);
	}

	// Awakening anew, the body still awaits.
	InMemoryCosmos cosmos(store);
	World& world = cosmos.life().create().world();
	REQUIRE(world.unborn().size() == 1);
	CHECK(world.unborn().front().get().id() == mark);
	CHECK(dynamic_cast<const Unborn*>(&world.welcome(seth_token)));
}


TEST_CASE("one bears at gates one keeps; the child is born of their host, who is ever his neighbour")
{
	InMemoryShared store;
	const DeviceToken child_token = DeviceToken::generate();
	{
		InMemoryCosmos cosmos(store);
		World& world = cosmos.life().create().world();

		const auto& anna = born(world, DeviceToken::generate());
		const auto& boris = born(world, DeviceToken::generate());
		const auto& unborn = dynamic_cast<const Unborn&>(world.welcome(child_token));

		// Not standing at gates, one does not bear.
		boris.wake();
		CHECK_THROWS_AS(world.bear(boris, unborn), std::logic_error);

		// There is no birth room: one is born through the gates.
		CHECK(anna.abode().rooms().size() == 3);
		CHECK_FALSE(anna.abode().room("Родильная"));

		// At Anna's gates Boris only waits: a stranger does not keep them, nor bears.
		const Gates& gates = anna.abode().gates();
		CHECK(gates.part() == matter::Room::Part::Inner);
		boris.contemplate(gates);
		CHECK_FALSE(gates.keeps(boris));
		CHECK_THROWS_AS(world.bear(boris, unborn), std::logic_error);
		boris.contemplate(boris.abode());
		admit_at_gates(world, anna, boris);
		regard_in_upper_room(world, anna, boris, matter::Dweller::Kind::Friend);

		// Many stand at the gates at once: Anna and her friend both keep them.
		anna.contemplate(gates);
		boris.contemplate(gates);
		CHECK(gates.dwells(anna));
		CHECK(gates.dwells(boris));
		CHECK(gates.keeps(boris));

		// Boris bears at Anna's gates: the child is born of Anna.
		const Man& child = world.bear(boris, unborn);
		CHECK(world.unborn().empty());
		CHECK(&world.man(world.vessel(child.Vessel::id())) == &child);
		CHECK(child.father(matter::Fatherhood::Line::Flesh) == static_cast<const Man*>(&anna));
		CHECK_FALSE(child.father(matter::Fatherhood::Line::Spirit));
		CHECK(std::dynamic_pointer_cast<const Neighbour>(child.abode().dweller(anna)));
		CHECK_FALSE(std::dynamic_pointer_cast<const Friend>(child.abode().dweller(anna)));
		REQUIRE(anna.abode().dweller(child));
		CHECK_FALSE(std::dynamic_pointer_cast<const Neighbour>(anna.abode().dweller(child)));
		CHECK_FALSE(child.abode().dwells(boris));
		CHECK_FALSE(boris.abode().dwells(child));

		// The father by flesh may be raised to a friend, never lowered below a neighbour.
		regard_in_upper_room(world, child, anna, matter::Dweller::Kind::Friend);
		CHECK(std::dynamic_pointer_cast<const Friend>(child.abode().dweller(anna)));
		CHECK_THROWS_AS(regard_in_upper_room(world, child, anna, matter::Dweller::Kind::Acquaintance),
						std::logic_error);
		regard_in_upper_room(world, child, anna, matter::Dweller::Kind::Neighbour);
	}

	// Awakening anew, the child knows his father again.
	InMemoryCosmos cosmos(store);
	World& world = cosmos.life().create().world();
	const Man& child = born(world, child_token);
	REQUIRE(child.father(matter::Fatherhood::Line::Flesh));
	CHECK(std::dynamic_pointer_cast<const Neighbour>(
		child.abode().dweller(*child.father(matter::Fatherhood::Line::Flesh))));
}


TEST_CASE("a man chooses his father by spirit, an unseen friend of all his spiritual line")
{
	using Line = matter::Fatherhood::Line;

	InMemoryShared store;
	const DeviceToken elder_token = DeviceToken::generate();
	const DeviceToken son_token = DeviceToken::generate();
	const DeviceToken grandson_token = DeviceToken::generate();
	{
		InMemoryCosmos cosmos(store);
		World& world = cosmos.life().create().world();
		const auto& elder = born(world, elder_token);
		const auto& son = born(world, son_token);
		const auto& grandson = born(world, grandson_token);
		const auto& other = born(world, DeviceToken::generate());

		CHECK_THROWS_AS(world.choose_father(son, son), std::logic_error);
		world.choose_father(son, elder);
		world.choose_father(grandson, son);
		CHECK(son.father(Line::Spirit) == static_cast<const Man*>(&elder));

		// A ring is not closed: one does not choose one's descendant by spirit.
		CHECK_THROWS_AS(world.choose_father(elder, grandson), std::logic_error);

		// The father by spirit is a friend in the abodes of all his line, entering every
		// room, yet unseen among their dwellers and not regarded anew.
		CHECK(std::dynamic_pointer_cast<const Friend>(son.abode().dweller(elder)));
		CHECK(std::dynamic_pointer_cast<const Friend>(grandson.abode().dweller(elder)));
		CHECK(grandson.abode().upper_room().dwells(elder));
		elder.wake();
		elder.contemplate(cell_of(grandson));
		for (const auto& dweller : son.abode().dwellers())
			CHECK(&dweller->man() != static_cast<const Man*>(&elder));
		CHECK_THROWS_AS(regard_in_upper_room(world, son, elder, matter::Dweller::Kind::Acquaintance),
						std::logic_error);

		// The grandson does not see into his father's or grandfather's abode.
		CHECK_FALSE(elder.abode().dwells(grandson));

		// His whole line, generation by generation.
		const std::vector<World::Descent> line = world.lineage(elder);
		REQUIRE(line.size() == 2);
		CHECK(&line[0].child.get() == static_cast<const Man*>(&son));
		CHECK(&line[0].father.get() == static_cast<const Man*>(&elder));
		CHECK(&line[1].child.get() == static_cast<const Man*>(&grandson));
		CHECK(&line[1].father.get() == static_cast<const Man*>(&son));

		// Choosing another father, the grandson leaves the elder's line.
		world.choose_father(grandson, other);
		CHECK_FALSE(grandson.abode().dwells(elder));
		CHECK_FALSE(grandson.abode().dwells(son));
		CHECK(world.lineage(elder).size() == 1);
		elder.sleep();
	}

	// Awakening anew, the line is as it was left.
	InMemoryCosmos cosmos(store);
	World& world = cosmos.life().create().world();
	const Man& elder = born(world, elder_token);
	const Man& son = born(world, son_token);
	const Man& grandson = born(world, grandson_token);
	CHECK(son.father(Line::Spirit) == &elder);
	CHECK(grandson.father(Line::Spirit) != &son);
	CHECK(world.lineage(elder).size() == 1);
}


TEST_CASE("whoever keeps the gates by his kind opens them and lets in an acquaintance of the host")
{
	InMemoryCosmos cosmos;
	World& world = cosmos.life().create().world();

	const auto& anna = born(world, DeviceToken::generate());
	const auto& boris = born(world, DeviceToken::generate());
	const auto& viktor = born(world, DeviceToken::generate());
	const auto& galina = born(world, DeviceToken::generate());
	const Gates& gates = anna.abode().gates();
	CHECK(gates.part() == matter::Room::Part::Inner);

	// Boris is Anna's acquaintance: standing in her gates, he does not keep them.
	admit_at_gates(world, anna, boris);
	boris.wake();
	viktor.wake();
	boris.contemplate(gates);
	viktor.contemplate(gates);
	CHECK_FALSE(gates.keeps(boris));
	CHECK_FALSE(gates.open());
	CHECK_THROWS_AS(admit(world, boris, viktor), std::logic_error);

	// A friend, he keeps them: they are open without Anna, and he lets Viktor in —
	// as Anna's acquaintance, not his.
	regard_in_upper_room(world, anna, boris, matter::Dweller::Kind::Friend);
	CHECK(gates.keeps(boris));
	CHECK(gates.open());
	admit(world, boris, viktor);
	REQUIRE(anna.abode().dweller(viktor));
	CHECK_FALSE(std::dynamic_pointer_cast<const Neighbour>(anna.abode().dweller(viktor)));
	CHECK_FALSE(boris.abode().dwells(viktor));
	CHECK_THROWS_AS(admit(world, boris, viktor), std::logic_error);

	// Away from the gates, one lets no one in.
	boris.contemplate(boris.abode());
	CHECK_FALSE(gates.open());
	galina.wake();
	galina.contemplate(gates);
	CHECK_THROWS_AS(admit(world, boris, galina), std::logic_error);

	// Set in the outer part, the gates are kept by a neighbour too.
	looking_at(world, anna, anna.abode().upper_room(), [&] { regard(world, anna, viktor, matter::Dweller::Kind::Neighbour); });
	arrange_in_upper_room(world, anna, gates, matter::Room::Part::Outer);
	viktor.contemplate(gates);
	CHECK(gates.keeps(viktor));
	admit(world, viktor, galina);
	CHECK(anna.abode().dweller(galina));
	CHECK_FALSE(viktor.abode().dwells(galina));
}


TEST_CASE("an exercise is named and done in approaches; an approach has repetitions; a weight is at most a ton")
{
	CHECK(Weight{0}.own());
	CHECK_FALSE(Weight{2'500}.own());
	CHECK_THROWS_AS(Weight{Weight::MaxGrams + 1}, std::invalid_argument);

	CHECK_THROWS_AS((Approach{Weight{60'000}, 0}), std::invalid_argument);
	CHECK_THROWS_AS((Approach{Weight{60'000}, 5, Approach::MaxRestSeconds + 1}), std::invalid_argument);
	CHECK_NOTHROW((Approach{Weight{60'000}, 1}));

	CHECK_THROWS_AS((Exercise{"", {Approach{Weight{0}, 5}}}), std::invalid_argument);
	CHECK_THROWS_AS((Exercise{"Присед", {}}), std::invalid_argument);
	CHECK_THROWS_AS((Exercise{std::string(Exercise::MaxNameLength + 1, 'a'), {Approach{Weight{0}, 5}}}),
					std::invalid_argument);

	CHECK_THROWS_AS((matter::Training{id::Word{1}, {}}), std::invalid_argument);
}


TEST_CASE("the testator wills a training of exercises; the novice fulfils it telling what he has done")
{
	InMemoryCosmos cosmos;
	World& world = cosmos.life().create().world();

	const auto& novice = born(world, DeviceToken::generate());
	const auto& testator = born(world, DeviceToken::generate());
	admit_at_gates(world, novice, testator);
	admit_at_gates(world, testator, novice);
	novice.supplicate(testator);
	const Shepherding& shepherding = testator.accept(*testator.supplication(novice));
	const Obedience& obedience = novice.obedience(testator);

	const std::vector<Exercise> willed{
		Exercise{"Жим лёжа", {Approach{Weight{60'000}, 10}, Approach{Weight{70'000}, 8}, Approach{Weight{80'000}, 6}}},
		Exercise{"Подтягивания", {Approach{Weight{0}, 8}, Approach{Weight{0}, 8}}},
	};

	// Only looking at the tie does the testator will a training.
	CHECK_THROWS_AS(train(world, testator, "Понедельник", willed), std::logic_error);
	testator.wake();
	testator.contemplate(room_in(testator, shepherding));
	CHECK_THROWS_AS(train(world, testator, "Пусто", {}), std::invalid_argument);
	const std::shared_ptr<const Training> monday = train(world, testator, "Понедельник", willed);
	const std::shared_ptr<const Training> tuesday = train(world, testator, "Вторник", {willed[1]});
	CHECK(monday->saying().body() == "Понедельник");
	CHECK(monday->exercises() == willed);

	// A training is a behest: the novice sees it, and it awaits him.
	REQUIRE(obedience.behests(novice).size() == 2);
	CHECK(std::dynamic_pointer_cast<const Training>(obedience.behests(novice).front()));
	CHECK(novice.abode().owed().size() == 2);

	// Fulfilled with nothing done, the deed tells no exercises; or with what he has done.
	novice.wake();
	novice.contemplate(room_in(novice, obedience));
	const std::shared_ptr<const Deed> as_willed = execute(world, novice, *tuesday);
	CHECK(as_willed->performed().empty());
	const std::vector<Exercise> done{
		Exercise{"Жим лёжа", {Approach{Weight{60'000}, 10}, Approach{Weight{70'000}, 7}, Approach{Weight{75'000}, 5}}},
		Exercise{"Подтягивания", {Approach{Weight{0}, 8}, Approach{Weight{0}, 6}}},
	};
	const std::shared_ptr<const Deed> told = execute(world, novice, *monday, Saying{"тяжело"}, done);
	CHECK(told->performed() == done);
	CHECK(told->saying().body() == "тяжело");
	CHECK(novice.abode().owed().empty());

	// A plain behest is fulfilled without exercises.
	testator.contemplate(room_in(testator, shepherding));
	const std::shared_ptr<const Behest> plain = will_here(world, testator, "Выспаться");
	CHECK_THROWS_AS(execute(world, novice, *plain, std::nullopt, done), std::logic_error);
	CHECK(execute(world, novice, *plain)->performed().empty());

	// Born anew from matter, the training and what was done are as they were.
	testator.sleep();
	novice.sleep();
	testator.wake();
	testator.contemplate(room_in(testator, shepherding));
	const auto words = world.contemplation(testator)->words();
	REQUIRE(words.size() == 6);
	const auto recalled = std::dynamic_pointer_cast<const Training>(words[0]);
	REQUIRE(recalled);
	CHECK(recalled.get() != monday.get());
	CHECK(recalled->exercises() == willed);
	CHECK_FALSE(std::dynamic_pointer_cast<const Training>(words[4]));
	const auto recalled_told = std::dynamic_pointer_cast<const Deed>(words[3]);
	REQUIRE(recalled_told);
	CHECK(recalled_told->performed() == done);
}


TEST_CASE("the novice does a training approach by approach: each begun and finished is an effort the training holds")
{
	InMemoryShared store;
	const DeviceToken novice_token = DeviceToken::generate();
	const DeviceToken testator_token = DeviceToken::generate();
	id::Word training_id{1};
	{
		InMemoryCosmos cosmos(store);
		World& world = cosmos.life().create().world();
		const auto& novice = born(world, novice_token);
		const auto& testator = born(world, testator_token);
		admit_at_gates(world, novice, testator);
		admit_at_gates(world, testator, novice);
		novice.supplicate(testator);
		const Shepherding& shepherding = testator.accept(*testator.supplication(novice));
		const Obedience& obedience = novice.obedience(testator);

		testator.wake();
		testator.contemplate(room_in(testator, shepherding));
		const std::shared_ptr<const Training> training = train(world, testator, "Вторник", {
			Exercise{"Присед", {Approach{Weight{100'000}, 5, 120}, Approach{Weight{100'000}, 5, 120}}},
			Exercise{"Подтягивания", {Approach{Weight{0}, 8, 90}}},
		});
		training_id = training->id();

		// Only looking at the tie does he begin; then one approach at a time.
		CHECK_THROWS_AS(begin(world, novice, *training, 0, 0), std::logic_error);
		novice.wake();
		novice.contemplate(room_in(novice, obedience));
		CHECK_THROWS_AS(begin(world, novice, *training, 2, 0), Refusal);
		CHECK_THROWS_AS(begin(world, novice, *training, 0, 3), Refusal);
		begin(world, novice, *training, 0, 0);
		REQUIRE(training->underway());
		CHECK(training->underway()->approach() == 0);
		CHECK_THROWS_AS(begin(world, novice, *training, 1, 0), std::logic_error);
		CHECK_THROWS_AS(execute(world, novice, *training), std::logic_error);

		cosmos.fake_time().set_instant(Timestamp{cosmos.fake_time().instant().value() + 40'000'000'000});
		const matter::Effort first = finish(world, novice, *training, Weight{100'000}, 5);
		CHECK_FALSE(training->underway());
		CHECK(first.finished().value() - first.begun().value() == 40'000'000'000);
		CHECK_THROWS_AS(finish(world, novice, *training, Weight{100'000}, 5), std::logic_error);

		// The training holds the effort: the testator, looking at the tie, sees it.
		REQUIRE(training->efforts().size() == 1);
		const auto seen = std::dynamic_pointer_cast<const Training>(shepherding.behests(testator).front());
		REQUIRE(seen);
		CHECK(seen->efforts().size() == 1);

		// An approach is done once; the approaches of an exercise in order, its
		// exercises in any; and, once the willed are done, one beyond them.
		CHECK_THROWS_AS(begin(world, novice, *training, 0, 0), std::logic_error);
		CHECK_THROWS_AS(begin(world, novice, *training, 0, 2), std::logic_error);
		begin(world, novice, *training, 1, 0);
		finish(world, novice, *training, Weight{0}, 6);
		begin(world, novice, *training, 0, 1);
		finish(world, novice, *training, Weight{90'000}, 4);
		CHECK_THROWS_AS(begin(world, novice, *training, 0, 4), Refusal);

		// Fulfilled, the deed tells what was done; then nothing more is begun.
		const std::shared_ptr<const Deed> deed = execute(world, novice, *training, Saying{"тяжело"});
		REQUIRE(deed->performed().size() == 2);
		CHECK(deed->performed()[0].name() == "Присед");
		REQUIRE(deed->performed()[0].approaches().size() == 2);
		CHECK(deed->performed()[0].approaches()[1] == Approach{Weight{90'000}, 4, 120});
		CHECK(deed->performed()[1].approaches()[0].repetitions() == 6);
		CHECK_THROWS_AS(begin(world, novice, *training, 0, 2), std::logic_error);
		testator.sleep();
		novice.sleep();
	}

	// Born anew from matter, the training holds its efforts.
	InMemoryCosmos cosmos(store);
	World& world = cosmos.life().create().world();
	const auto& testator = born(world, testator_token);
	const auto& novice = born(world, novice_token);
	testator.wake();
	testator.contemplate(room_in(testator, testator.shepherding(novice)));
	const auto recalled = std::dynamic_pointer_cast<const Training>(world.contemplation(testator)->words()[0]);
	REQUIRE(recalled);
	CHECK(recalled->id() == training_id);
	REQUIRE(recalled->efforts().size() == 3);
	CHECK(recalled->efforts()[0].repetitions() == 5);
}


TEST_CASE("an approach underway is the training's, one at a time in each, and outlives the world")
{
	InMemoryShared store;
	const DeviceToken novice_token = DeviceToken::generate();
	const DeviceToken testator_token = DeviceToken::generate();
	id::Word monday_id{1};
	{
		InMemoryCosmos cosmos(store);
		World& world = cosmos.life().create().world();
		const Man& novice = born(world, novice_token);
		const Man& testator = born(world, testator_token);
		admit_at_gates(world, novice, testator);
		admit_at_gates(world, testator, novice);
		novice.supplicate(testator);
		const Shepherding& shepherding = testator.accept(*testator.supplication(novice));

		testator.wake();
		testator.contemplate(room_in(testator, shepherding));
		const auto monday = train(world, testator, "Понедельник", {Exercise{"Присед", {Approach{Weight{100'000}, 5}}}});
		const auto tuesday = train(world, testator, "Вторник", {Exercise{"Жим", {Approach{Weight{60'000}, 8}}}});
		monday_id = monday->id();

		// Each training has its own approach underway, and only one.
		novice.wake();
		novice.contemplate(room_in(novice, novice.obedience(testator)));
		begin(world, novice, *monday, 0, 0);
		begin(world, novice, *tuesday, 0, 0);
		CHECK(monday->underway());
		CHECK(tuesday->underway());
		CHECK_THROWS_AS(begin(world, novice, *monday, 0, 0), AnotherApproachUnderway);
		testator.sleep();
		novice.sleep();
	}

	// Born anew from matter, the training holds its approach underway, and it is finished.
	InMemoryCosmos cosmos(store);
	World& world = cosmos.life().create().world();
	const Man& testator = born(world, testator_token);
	const Man& novice = born(world, novice_token);
	novice.wake();
	novice.contemplate(room_in(novice, novice.obedience(testator)));
	std::shared_ptr<const Training> monday;
	for (const auto& word : world.contemplation(novice)->words()) {
		if (word->id() == monday_id)
			monday = std::dynamic_pointer_cast<const Training>(word);
	}
	REQUIRE(monday);
	REQUIRE(monday->underway());
	CHECK(monday->underway()->approach() == 0);
	finish(world, novice, *monday, Weight{100'000}, 5);
	CHECK_FALSE(monday->underway());
	CHECK(monday->efforts().size() == 1);
}


TEST_CASE("in the novice's cell of a tie, whoever stands there does what the novice does, and it is the novice's")
{
	InMemoryCosmos cosmos;
	World& world = cosmos.life().create().world();
	const auto& novice = born(world, DeviceToken::generate());
	const auto& testator = born(world, DeviceToken::generate());
	const auto& friend_ = born(world, DeviceToken::generate());
	admit_at_gates(world, novice, testator);
	admit_at_gates(world, testator, novice);
	novice.supplicate(testator);
	const Shepherding& shepherding = testator.accept(*testator.supplication(novice));
	const Obedience& obedience = novice.obedience(testator);
	admit_at_gates(world, novice, friend_);
	regard_in_upper_room(world, novice, friend_, matter::Dweller::Kind::Friend);

	testator.wake();
	testator.contemplate(room_in(testator, shepherding));
	const std::shared_ptr<const Training> training
		= train(world, testator, "Среда", {Exercise{"Присед", {Approach{Weight{100'000}, 5}}}});

	// In his own cell the testator does not train nor fulfil: that is the novice's.
	CHECK_THROWS_AS(begin(world, testator, *training, 0, 0), OnlyNoviceTrains);
	CHECK_THROWS_AS(execute(world, testator, *training), OnlyNoviceFulfils);

	// The friend, in the novice's cell, begins an approach of the novice's training.
	friend_.wake();
	friend_.contemplate(room_in(novice, obedience));
	begin(world, friend_, *training, 0, 0);
	REQUIRE(training->underway());
	CHECK(training->underway()->training() == training->id());

	const matter::Effort effort = finish(world, friend_, *training, Weight{100'000}, 5);
	CHECK_FALSE(training->underway());
	CHECK(effort.repetitions() == 5);
	REQUIRE(training->efforts().size() == 1);

	// And he fulfils it: the deed is the novice's.
	const std::shared_ptr<const Deed> deed = execute(world, friend_, *training);
	CHECK(&deed->author() == static_cast<const Soul*>(&novice));
	REQUIRE(deed->performed().size() == 1);
	CHECK(novice.abode().owed().empty());
}


TEST_CASE("the world awakens though what is kept disagrees: it passes it over, tells it, and makes fatherhood whole")
{
	InMemoryShared store;
	const DeviceToken anna_token = DeviceToken::generate();
	const DeviceToken boris_token = DeviceToken::generate();
	std::optional<id::Soul> kept_boris;
	std::optional<id::Place> kept_anna_abode;
	std::optional<id::Soul> kept_elder;
	{
		InMemoryCosmos cosmos(store);
		World& world = cosmos.life().create().world();
		const auto& anna = born(world, anna_token);
		const auto& boris = born(world, boris_token);
		admit_at_gates(world, anna, boris);
		admit_at_gates(world, boris, anna);
		boris.supplicate(anna);
		anna.accept(*anna.supplication(boris));
		kept_boris = boris.Soul::id();
		kept_anna_abode = anna.abode().id();
		kept_elder = forefather(world).Soul::id();
		CHECK(world.unrecollected().empty());
	}

	const id::Soul boris_id = *kept_boris;
	const id::Place anna_abode = *kept_anna_abode;
	const id::Soul elder_id = *kept_elder;

	// What is kept no longer agrees: Boris's body has lost its soul, a body is joined to a
	// soul never kept, a dweller dwells where no one keeps the abode, and Anna's father
	// no longer dwells in her abode.
	std::erase_if(store.embodiments, [&](const matter::Embodiment& e) { return e.soul() == boris_id; });
	store.embodiments.emplace_back(id::Soul{9999}, id::Vessel{9998});
	store.dwellers.emplace_back(id::Place{12345}, elder_id, matter::Dweller::Kind::Friend);
	std::erase_if(store.dwellers, [&](const matter::Dweller& d) { return d.abode() == anna_abode && d.soul() == elder_id; });

	// The world awakens all the same, and says what it passed over and what it made whole.
	InMemoryCosmos cosmos(store);
	World& world = cosmos.life().create().world();
	const auto told = [&](const std::string_view what) {
		return std::any_of(world.unrecollected().begin(), world.unrecollected().end(),
						   [&](const std::string& said) { return said.find(what) != std::string::npos; });
	};
	CHECK(told("embodiment of soul 9999"));
	CHECK(told("soul " + std::to_string(boris_id.value()) + " ("));  // Boris's soul, in no body now
	CHECK(told("has no body"));
	CHECK(told("tie "));                           // Anna's tie with Boris, who does not live
	CHECK(told("dweller " + std::to_string(boris_id.value())));
	CHECK(told("of abode 12345"));
	CHECK(told("fatherhood of " + std::to_string(elder_id.value()) + " over " + std::to_string(boris_id.value())));
	CHECK(told("made a neighbour of his child"));

	const Man& anna = born(world, anna_token);
	CHECK(anna.abode().dwells(forefather(world)));
	CHECK(std::any_of(store.dwellers.begin(), store.dwellers.end(), [&](const matter::Dweller& d) {
		return d.abode() == anna_abode && d.soul() == elder_id;
	}));
	// Anna's tie with Boris is not recollected: no room of it stands in her abode.
	CHECK(anna.abode().rooms().size() == 3);
}


TEST_CASE("a refusal is of its kind, the rule of the being that keeps it, in its own words")
{
	InMemoryCosmos cosmos;
	World& world = cosmos.life().create().world();
	const auto& man = born(world, DeviceToken::generate());


	// One does not ask oneself; one is not one's own father.
	CHECK_THROWS_AS(man.supplicate(man), CannotSupplicateOneself);
	CHECK_THROWS_AS(world.choose_father(man, man), NotOwnFather);

	// A refusal is told in words too, and is caught as any logic_error.
	try {
		world.choose_father(man, man);
	} catch (const std::logic_error& error) {
		CHECK(std::string{error.what()} == "one is not one's own father");
	}
}


TEST_CASE("one writes where one stands: a letter in one's word, a behest in a tie one wills in, nothing elsewhere")
{
	InMemoryCosmos cosmos;
	World& world = cosmos.life().create().world();
	const auto& testator = born(world, DeviceToken::generate());
	const auto& novice = born(world, DeviceToken::generate());
	admit_at_gates(world, testator, novice);
	admit_at_gates(world, novice, testator);
	novice.supplicate(testator);
	const Tie& tie = dynamic_cast<const Tie&>(testator.accept(*testator.supplication(novice)));


	// In one's own word: a letter, written by oneself.
	testator.contemplate(cell_of(testator));
	CHECK(writes_here(world, testator));
	const auto letter = write(world, testator, Saying{"запись"});
	CHECK(dynamic_cast<const Letter*>(letter.get()));
	CHECK(&letter->author() == static_cast<const Soul*>(&testator));

	// At the gates and in the upper room no one writes: they show no words.
	for (const Room* room : {static_cast<const Room*>(&testator.abode().gates()),
							 static_cast<const Room*>(&testator.abode().upper_room())}) {
		testator.contemplate(*room);
		CHECK_FALSE(writes_here(world, testator));
		CHECK_THROWS_AS(write(world, testator, Saying{"нет"}), StandInCell);
	}

	// In the tie the testator wills; the behest is his, its counterpart the novice.
	testator.contemplate(room_in(testator, tie));
	CHECK(writes_here(world, testator));
	const auto behest = write(world, testator, Saying{"задание"});
	CHECK(dynamic_cast<const Behest*>(behest.get()));
	CHECK(&behest->author() == static_cast<const Soul*>(&testator));
	CHECK(&tie.counterpart(testator) == static_cast<const Man*>(&novice));
	CHECK(&tie.counterpart(novice) == static_cast<const Man*>(&testator));

	// The novice, in the same tie, does not will: the world refuses him.
	novice.contemplate(room_in(novice, tie));
	CHECK_FALSE(writes_here(world, novice));
	CHECK_THROWS_AS(write(world, novice, Saying{"нет"}), OnlyTestatorWills);
	CHECK_THROWS_AS(train(world, novice, Saying{"нет"}, {Exercise{"Жим", {Approach{Weight{0}, 5}}}}), OnlyTestatorWills);

	// Standing in no tie, one wills no training.
	testator.contemplate(cell_of(testator));
	CHECK_THROWS_AS(train(world, testator, Saying{"нет"}, {Exercise{"Жим", {Approach{Weight{0}, 5}}}}), WillOnlyInContemplatedTie);

	// A friend of the testator, entering his cell of the tie, wills there as the testator.
	const auto& friend_ = born(world, DeviceToken::generate());
	admit_at_gates(world, testator, friend_);
	regard_in_upper_room(world, testator, friend_, matter::Dweller::Kind::Friend);
	friend_.wake();
	friend_.contemplate(room_in(testator, tie));
	CHECK(writes_here(world, friend_));
	const auto willed = will_here(world, friend_, "от друга");
	REQUIRE(willed);
	CHECK(&willed->author() == static_cast<const Soul*>(&testator));
	CHECK(&willed->tie() == &tie);
	const auto trained = train(world, friend_, Saying{"Тренировка"}, {Exercise{"Жим", {Approach{Weight{0}, 5}}}});
	CHECK(&trained->author() == static_cast<const Soul*>(&testator));
}
