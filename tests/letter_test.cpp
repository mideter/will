#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "domain_fakes.h"

#include "acts/creation.h"
#include "matter/dating.h"
#include "matter/execution.h"
#include "matter/placement.h"
#include "beings/immanents/tie.h"
#include "matter/tie.h"
#include "matter/utterance.h"
#include "beings/letter.h"
#include "beings/deed.h"
#include "beings/immanents/novice.h"
#include "beings/immanents/testator.h"
#include "identity/word.h"
#include "identity/tie.h"
#include "identity/place.h"
#include "values/saying.h"

#include <stdexcept>
#include <string>


using namespace will::domain;
using namespace will::domain::test;


TEST_CASE("id::Word requires positive value")
{
	CHECK(id::Word{1}.value() == 1);
	CHECK_THROWS_AS(id::Word{0}, std::invalid_argument);
}


TEST_CASE("matter::Utterance stores fields")
{
	const matter::Utterance utterance{id::Word{1}, id::Soul{7}, "hello"};
	CHECK(utterance.id() == id::Word{1});
	CHECK(utterance.author() == id::Soul{7});
	CHECK(utterance.saying().body() == "hello");
}


TEST_CASE("matter::Utterance rejects invalid construction")
{
	CHECK_THROWS_AS((matter::Utterance{id::Word{1}, id::Soul{0}, "x"}), std::invalid_argument);
	CHECK_THROWS_AS((Saying{""}), std::invalid_argument);
	CHECK_THROWS_AS((Saying{std::string(Saying::MaxBodyLength + 1, 'a')}), std::invalid_argument);
}


TEST_CASE("matter::Utterance accepts max body length")
{
	const matter::Utterance max_body{id::Word{1}, id::Soul{1}, std::string(Saying::MaxBodyLength, 'a')};
	CHECK(max_body.saying().body().size() == Saying::MaxBodyLength);
}


TEST_CASE("Letter is born from three projections with living place and author")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& man = world.welcome(DeviceToken::generate());
	const auto& witness = static_cast<const Witness&>(man);

	const matter::Utterance utterance{id::Word{1}, man.Soul::id(), "hello"};
	const matter::Placement placement{id::Word{1}, witness.abode().id()};
	const matter::Dating dating{id::Word{1}, Timestamp{100}};
	const Letter letter{utterance, placement, dating};

	CHECK(letter.id() == id::Word{1});
	CHECK(&letter.place() == &witness.abode());
	CHECK(&letter.author() == &static_cast<const Soul&>(man));
	CHECK(letter.saying().body() == "hello");
	CHECK(letter.created_at() == Timestamp{100});
}


TEST_CASE("Letter rejects unknown place")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& man = world.welcome(DeviceToken::generate());

	const matter::Utterance utterance{id::Word{1}, man.Soul::id(), "x"};
	const matter::Placement placement{id::Word{1}, id::Place{999}};
	const matter::Dating dating{id::Word{1}, Timestamp{0}};
	CHECK_THROWS_AS((Letter{utterance, placement, dating}), std::logic_error);
}


TEST_CASE("Letter rejects mismatched projection ids")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& man = world.welcome(DeviceToken::generate());
	const auto& witness = static_cast<const Witness&>(man);

	const matter::Utterance utterance{id::Word{1}, man.Soul::id(), "x"};
	const matter::Placement placement{id::Word{2}, witness.abode().id()};
	const matter::Dating dating{id::Word{1}, Timestamp{0}};
	CHECK_THROWS_AS((Letter{utterance, placement, dating}), std::invalid_argument);
}


TEST_CASE("Deed is a word of will in a living Tie")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const auto& testator = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));
	const auto& novice = static_cast<const Novice&>(world.welcome(DeviceToken::generate()));
	const id::Word did{3};
	const id::Tie oid{9};
	const Tie tie{matter::Tie{oid, testator.Soul::id(), novice.Soul::id()}};

	const matter::Utterance utterance{did, testator.Soul::id(), "do this"};
	const matter::Placement placement{did, tie.id()};
	const matter::Dating dating{did, Timestamp{50}};

	const Deed deed{utterance, placement, dating};
	CHECK(deed.id() == did);
	CHECK(&deed.tie() == &tie);
	CHECK(&deed.tie().testator() == &testator);
	CHECK(&deed.tie().novice() == &novice);
	CHECK(deed.saying().body() == "do this");
	CHECK(deed.created_at() == Timestamp{50});
	CHECK(deed.open());
	CHECK_FALSE(deed.executed());

	const Deed done{utterance, placement, dating, matter::Execution{did, Timestamp{70}}};
	CHECK(done.executed());
	CHECK_FALSE(done.open());
	CHECK(done.executed_at() == Timestamp{70});

	CHECK_THROWS_AS((Deed{utterance, placement, dating, matter::Execution{id::Word{4}, Timestamp{70}}}),
					std::invalid_argument);
	CHECK_THROWS_AS((Deed{utterance, matter::Placement{id::Word{4}, tie.id()}, dating}), std::invalid_argument);
}


TEST_CASE("Deed rejects a place that is not a Tie and an author who is not its testator")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const auto& testator = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));
	const auto& novice = static_cast<const Novice&>(world.welcome(DeviceToken::generate()));
	const id::Word did{3};
	const Tie tie{matter::Tie{id::Tie{9}, testator.Soul::id(), novice.Soul::id()}};
	const matter::Dating dating{did, Timestamp{50}};

	const matter::Utterance by_testator{did, testator.Soul::id(), "do this"};
	CHECK_THROWS_AS((Deed{by_testator, matter::Placement{did, testator.abode().id()}, dating}),
					std::invalid_argument);

	const matter::Utterance by_novice{did, novice.Soul::id(), "do this"};
	CHECK_THROWS_AS((Deed{by_novice, matter::Placement{did, tie.id()}, dating}), std::invalid_argument);
}
