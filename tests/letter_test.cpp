#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "domain_fakes.h"

#include "acts/creation.h"
#include "acts/dating.h"
#include "acts/placement.h"
#include "beings/immanents/tie.h"
#include "acts/tying.h"
#include "acts/utterance.h"
#include "beings/letter.h"
#include "beings/deed.h"
#include "beings/immanents/novice.h"
#include "beings/immanents/testator.h"
#include "identity/letter.h"
#include "identity/tie.h"
#include "identity/place.h"
#include "identity/deed.h"
#include "values/saying.h"

#include <stdexcept>
#include <string>


using namespace will::domain;
using namespace will::domain::test;


TEST_CASE("id::Letter requires positive value")
{
	CHECK(id::Letter{1}.value() == 1);
	CHECK_THROWS_AS(id::Letter{0}, std::invalid_argument);
}


TEST_CASE("Utterance stores fields")
{
	const Utterance utterance{id::Letter{1}, id::Soul{7}, "hello"};
	CHECK(utterance.id() == id::Letter{1});
	CHECK(utterance.author() == id::Soul{7});
	CHECK(utterance.saying().body() == "hello");
}


TEST_CASE("Utterance rejects invalid construction")
{
	CHECK_THROWS_AS((Utterance{id::Letter{1}, id::Soul{0}, "x"}), std::invalid_argument);
	CHECK_THROWS_AS((Saying{""}), std::invalid_argument);
	CHECK_THROWS_AS((Saying{std::string(Saying::MaxBodyLength + 1, 'a')}), std::invalid_argument);
}


TEST_CASE("Utterance accepts max body length")
{
	const Utterance max_body{id::Letter{1}, id::Soul{1}, std::string(Saying::MaxBodyLength, 'a')};
	CHECK(max_body.saying().body().size() == Saying::MaxBodyLength);
}


TEST_CASE("Letter is born from three projections with living place and author")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& man = world.welcome(DeviceToken::generate());
	const auto& witness = static_cast<const Witness&>(man);

	const Utterance utterance{id::Letter{1}, man.Soul::id(), "hello"};
	const Placement placement{id::Letter{1}, witness.abode().id()};
	const Dating dating{id::Letter{1}, Timestamp{100}};
	const Letter letter{utterance, placement, dating};

	CHECK(letter.id() == id::Letter{1});
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

	const Utterance utterance{id::Letter{1}, man.Soul::id(), "x"};
	const Placement placement{id::Letter{1}, id::Place{999}};
	const Dating dating{id::Letter{1}, Timestamp{0}};
	CHECK_THROWS_AS((Letter{utterance, placement, dating}), std::logic_error);
}


TEST_CASE("Letter rejects mismatched projection ids")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& man = world.welcome(DeviceToken::generate());
	const auto& witness = static_cast<const Witness&>(man);

	const Utterance utterance{id::Letter{1}, man.Soul::id(), "x"};
	const Placement placement{id::Letter{2}, witness.abode().id()};
	const Dating dating{id::Letter{1}, Timestamp{0}};
	CHECK_THROWS_AS((Letter{utterance, placement, dating}), std::invalid_argument);
}


TEST_CASE("id::Deed requires positive value")
{
	CHECK(id::Deed{4}.value() == 4);
	CHECK_THROWS_AS(id::Deed{0}, std::invalid_argument);
}


TEST_CASE("Deed is a word of will in a living Tie")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const auto& testator = static_cast<const Testator&>(world.welcome(DeviceToken::generate()));
	const auto& novice = static_cast<const Novice&>(world.welcome(DeviceToken::generate()));
	const id::Deed did{3};
	const id::Tie oid{9};
	const Tie tie{Tying{oid, testator.Soul::id(), novice.Soul::id()}};

	const Deed deed{did, tie, "do this", Timestamp{50}};
	CHECK(deed.id() == did);
	CHECK(&deed.tie() == &tie);
	CHECK(&deed.tie().testator() == &testator);
	CHECK(&deed.tie().novice() == &novice);
	CHECK(deed.saying().body() == "do this");
	CHECK(deed.created_at() == Timestamp{50});
	CHECK(deed.open());
	CHECK_FALSE(deed.executed());
	CHECK_FALSE(deed.cancelled());

	CHECK_THROWS_AS((Deed{id::Deed{6}, tie, "x", Timestamp{1}, Timestamp{2}, Timestamp{3}}),
					std::invalid_argument);
}
