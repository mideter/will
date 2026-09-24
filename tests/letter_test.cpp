#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "domain_fakes.h"

#include "acts/creation.h"
#include "acts/dating.h"
#include "acts/placement.h"
#include "acts/tie.h"
#include "acts/tying.h"
#include "acts/utterance.h"
#include "beings/letter.h"
#include "beings/deed.h"
#include "beings/novice.h"
#include "beings/testator.h"
#include "identity/letter.h"
#include "identity/tie.h"
#include "identity/place.h"
#include "identity/deed.h"
#include "values/word.h"

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
	const Utterance utterance{id::Letter{1}, id::Soul{7}, Word{"hello"}};
	CHECK(utterance.id() == id::Letter{1});
	CHECK(utterance.author() == id::Soul{7});
	CHECK(utterance.word().body() == "hello");
}


TEST_CASE("Utterance rejects invalid construction")
{
	CHECK_THROWS_AS((Utterance{id::Letter{1}, id::Soul{0}, Word{"x"}}), std::invalid_argument);
	CHECK_THROWS_AS((Word{""}), std::invalid_argument);
	CHECK_THROWS_AS((Word{std::string(Word::MaxBodyLength + 1, 'a')}), std::invalid_argument);
}


TEST_CASE("Utterance accepts max body length")
{
	const Utterance max_body{id::Letter{1}, id::Soul{1}, Word{std::string(Word::MaxBodyLength, 'a')}};
	CHECK(max_body.word().body().size() == Word::MaxBodyLength);
}


TEST_CASE("Letter is born from three projections with living place and author")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& man = world.welcome(DeviceToken::generate());
	const auto& witness = static_cast<const Witness&>(man);

	const Utterance utterance{id::Letter{1}, man.Soul::id(), Word{"hello"}};
	const Placement placement{id::Letter{1}, witness.abode().id()};
	const Dating dating{id::Letter{1}, Timestamp{100}};
	const Letter letter{utterance, placement, dating};

	CHECK(letter.id() == id::Letter{1});
	CHECK(&letter.place() == &witness.abode());
	CHECK(&letter.author() == &static_cast<const Soul&>(man));
	CHECK(letter.body() == "hello");
	CHECK(letter.created_at() == Timestamp{100});
}


TEST_CASE("Letter rejects unknown place")
{
	InMemoryCosmos cosmos;
	Creation creation(cosmos.eternity(), cosmos.spatiality(), cosmos.temporality());
	World& world = creation.world();

	const Man& man = world.welcome(DeviceToken::generate());

	const Utterance utterance{id::Letter{1}, man.Soul::id(), Word{"x"}};
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

	const Utterance utterance{id::Letter{1}, man.Soul::id(), Word{"x"}};
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

	const Deed deed{did, tie, Word{"do this"}, Timestamp{50}};
	CHECK(deed.id() == did);
	CHECK(&deed.tie() == &tie);
	CHECK(&deed.tie().testator() == &testator);
	CHECK(&deed.tie().novice() == &novice);
	CHECK(deed.body() == "do this");
	CHECK(deed.created_at() == Timestamp{50});
	CHECK(deed.open());
	CHECK_FALSE(deed.executed());
	CHECK_FALSE(deed.cancelled());

	CHECK_THROWS_AS((Deed{id::Deed{6}, tie, Word{"x"}, Timestamp{1}, Timestamp{2}, Timestamp{3}}),
					std::invalid_argument);
}
