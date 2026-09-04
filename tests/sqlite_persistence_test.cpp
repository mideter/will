#include "entities/earth.h"
#include "entities/heaven.h"
#include "entities/dead_vessel.h"
#include "sqlite_database.h"
#include "sqlite_store.h"
#include "sqlite_letter_repository_impl.h"

#include "ids/abode.h"
#include "values/timestamp.h"
#include "ids/soul.h"
#include "values/soul_name.h"

#include <cassert>
#include <cstdlib>
#include <string>
#include <unistd.h>


namespace {


will::domain::Soul register_soul(will::domain::Heaven& heaven, will::domain::Earth& earth,
                               will::domain::Eternity& eternity, const std::string_view device_token,
                               const will::domain::SoulName name)
{
    auto [soul, vessel] = eternity.insert_soul_with_vessel(device_token, name);
    heaven.insert(soul);
    earth.insert(std::move(vessel));
    return soul;
}


} // namespace


int main()
{
    using namespace will;
    using namespace will::domain;

    const std::string db_path = "/tmp/will-sqlite-persistence-test-" + std::to_string(getpid()) + ".db";
    ::unlink(db_path.c_str());

    std::optional<id::Soul> soul_a_id;
    std::optional<id::Soul> soul_b_id;
    std::optional<id::Soul> created_id;
    const std::string token = "abcd1234abcd1234abcd1234abcd1234";

    {
        SqliteDatabase database(db_path);
        SqliteStore store(database);
        SqliteLetterRepositoryImpl letters(database);
        Heaven heaven(store);
        Earth earth(heaven);

        const Soul soul_a = register_soul(heaven, earth, store, "aaaa1234aaaa1234aaaa1234aaaa1234",
                                          *SoulName::parse("nameaaaa"));
        const Soul soul_b = register_soul(heaven, earth, store, "bbbb1234bbbb1234bbbb1234bbbb1234",
                                          *SoulName::parse("namebbbb"));
        soul_a_id = soul_a.id();
        soul_b_id = soul_b.id();

        const id::Abode abode = id::Abode::global();
        letters.append(abode, soul_a.id(), "from-peer", Timestamp{1000});
        letters.append(abode, soul_b.id(), "from-me", Timestamp{2000});

        const auto rows = letters.load_last(abode, 10);
        assert(rows.size() == 2);
        assert(rows[0].body() == "from-peer");
        assert(rows[0].author_id() == soul_a.id());
        assert(heaven.find_by_id(rows[0].author_id())->name() == *SoulName::parse("nameaaaa"));
        assert(rows[1].body() == "from-me");
        assert(rows[1].author_id() == soul_b.id());
        assert(heaven.find_by_id(rows[1].author_id())->name() == *SoulName::parse("namebbbb"));

        const Soul created = register_soul(heaven, earth, store, token, *SoulName::parse("abcdefgh"));
        created_id = created.id();
        assert(created.id().value() > 0);
        assert(earth.soul_id_for_dead(DeadVessel{token}) == created.id());
        assert(created.name() == *SoulName::parse("abcdefgh"));

        assert(earth.find_by_dead(DeadVessel{token}).has_value());
        assert(heaven.find_by_id(created.id()).has_value());
    }

    {
        SqliteDatabase database(db_path);
        SqliteStore store(database);
        Heaven heaven(store);
        Earth earth(heaven);

        assert(earth.soul_id_for_dead(DeadVessel{token}) == *created_id);
        assert(heaven.find_by_id(*created_id)->name() == *SoulName::parse("abcdefgh"));

        const std::optional<Soul> a = heaven.find_by_id(*soul_a_id);
        assert(a.has_value());
        assert(a->name() == *SoulName::parse("nameaaaa"));
        assert(earth.soul_id_for_dead(DeadVessel{"aaaa1234aaaa1234aaaa1234aaaa1234"}) == *soul_a_id);

        const std::optional<Soul> b = heaven.find_by_id(*soul_b_id);
        assert(b.has_value());
        assert(b->name() == *SoulName::parse("namebbbb"));

        assert(!heaven.find_by_id(id::Soul{999999}).has_value());
        assert(!earth.find_by_dead(DeadVessel{"ffffffffffffffffffffffffffffffff"}).has_value());
    }

    ::unlink(db_path.c_str());
    return EXIT_SUCCESS;
}
