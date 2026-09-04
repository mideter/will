#include "entities/earth.h"
#include "entities/heaven.h"
#include "entities/dead_vessel.h"
#include "sqlite_database.h"
#include "sqlite_store.h"
#include "sqlite_letter_repository_impl.h"

#include "ids/abode.h"
#include "values/timestamp.h"
#include "ids/god.h"
#include "values/god_name.h"

#include <cassert>
#include <cstdlib>
#include <string>
#include <unistd.h>


namespace {


will::domain::God register_god(will::domain::Heaven& heaven, will::domain::Earth& earth,
                               will::domain::Eternity& eternity, const std::string_view device_token,
                               const will::domain::GodName name)
{
    auto [god, vessel] = eternity.insert_god_with_vessel(device_token, name);
    heaven.insert(god);
    earth.insert(std::move(vessel));
    return god;
}


} // namespace


int main()
{
    using namespace will;
    using namespace will::domain;

    const std::string db_path = "/tmp/will-sqlite-persistence-test-" + std::to_string(getpid()) + ".db";
    ::unlink(db_path.c_str());

    std::optional<id::God> god_a_id;
    std::optional<id::God> god_b_id;
    std::optional<id::God> created_id;
    const std::string token = "abcd1234abcd1234abcd1234abcd1234";

    {
        SqliteDatabase database(db_path);
        SqliteStore store(database);
        SqliteLetterRepositoryImpl letters(database);
        Heaven heaven(store);
        Earth earth(heaven);

        const God god_a = register_god(heaven, earth, store, "aaaa1234aaaa1234aaaa1234aaaa1234",
                                       *GodName::parse("nameaaaa"));
        const God god_b = register_god(heaven, earth, store, "bbbb1234bbbb1234bbbb1234bbbb1234",
                                       *GodName::parse("namebbbb"));
        god_a_id = god_a.id();
        god_b_id = god_b.id();

        const id::Abode abode = id::Abode::global();
        letters.append(abode, god_a.id(), "from-peer", Timestamp{1000});
        letters.append(abode, god_b.id(), "from-me", Timestamp{2000});

        const auto rows = letters.load_last(abode, 10);
        assert(rows.size() == 2);
        assert(rows[0].body() == "from-peer");
        assert(rows[0].author_id() == god_a.id());
        assert(heaven.find_by_id(rows[0].author_id())->name() == *GodName::parse("nameaaaa"));
        assert(rows[1].body() == "from-me");
        assert(rows[1].author_id() == god_b.id());
        assert(heaven.find_by_id(rows[1].author_id())->name() == *GodName::parse("namebbbb"));

        const God created = register_god(heaven, earth, store, token, *GodName::parse("abcdefgh"));
        created_id = created.id();
        assert(created.id().value() > 0);
        assert(earth.god_id_for_dead(DeadVessel{token}) == created.id());
        assert(created.name() == *GodName::parse("abcdefgh"));

        assert(earth.find_by_dead(DeadVessel{token}).has_value());
        assert(heaven.find_by_id(created.id()).has_value());
    }

    {
        SqliteDatabase database(db_path);
        SqliteStore store(database);
        Heaven heaven(store);
        Earth earth(heaven);

        assert(earth.god_id_for_dead(DeadVessel{token}) == *created_id);
        assert(heaven.find_by_id(*created_id)->name() == *GodName::parse("abcdefgh"));

        const std::optional<God> a = heaven.find_by_id(*god_a_id);
        assert(a.has_value());
        assert(a->name() == *GodName::parse("nameaaaa"));
        assert(earth.god_id_for_dead(DeadVessel{"aaaa1234aaaa1234aaaa1234aaaa1234"}) == *god_a_id);

        const std::optional<God> b = heaven.find_by_id(*god_b_id);
        assert(b.has_value());
        assert(b->name() == *GodName::parse("namebbbb"));

        assert(!heaven.find_by_id(id::God{999999}).has_value());
        assert(!earth.find_by_dead(DeadVessel{"ffffffffffffffffffffffffffffffff"}).has_value());
    }

    ::unlink(db_path.c_str());
    return EXIT_SUCCESS;
}
