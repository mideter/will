#include "entities/earth.h"
#include "entities/heaven.h"
#include "entities/man.h"
#include "sqlite_database.h"
#include "sqlite_store.h"
#include "sqlite_letter_repository_impl.h"

#include "identity/abode.h"
#include "values/device_token.h"
#include "values/timestamp.h"
#include "identity/soul.h"
#include "values/soul_name.h"

#include <cassert>
#include <cstdlib>
#include <string>
#include <unistd.h>


namespace {


will::domain::Soul register_soul(will::domain::Heaven& heaven, will::domain::Earth& earth,
                               will::domain::Eternity& eternity, const will::domain::DeviceToken& token,
                               const will::domain::SoulName name)
{
    will::domain::ManBirth birth = eternity.insert_man(token, name);
    heaven.insert(birth.soul);
    earth.insert(std::move(birth.vessel));
    earth.insert(birth.man);
    return birth.soul;
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
    const std::string token_text = "abcd1234abcd1234abcd1234abcd1234";

    {
        SqliteDatabase database(db_path);
        SqliteStore store(database);
        SqliteLetterRepositoryImpl letters(database);
        Heaven heaven(store);
        Earth earth(heaven);

        const DeviceToken token_a = *DeviceToken::parse("aaaa1234aaaa1234aaaa1234aaaa1234");
        const DeviceToken token_b = *DeviceToken::parse("bbbb1234bbbb1234bbbb1234bbbb1234");
        const DeviceToken token_created = *DeviceToken::parse(token_text);

        const Soul soul_a = register_soul(heaven, earth, store, token_a, *SoulName::parse("nameaaaa"));
        const Soul soul_b = register_soul(heaven, earth, store, token_b, *SoulName::parse("namebbbb"));
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

        const Soul created = register_soul(heaven, earth, store, token_created, *SoulName::parse("abcdefgh"));
        created_id = created.id();
        assert(created.id().value() > 0);
        assert(earth.soul_id_for_token(token_created) == created.id());
        assert(created.name() == *SoulName::parse("abcdefgh"));

        assert(earth.find_man_by_token(token_created).has_value());
        assert(heaven.find_by_id(created.id()).has_value());
    }

    {
        SqliteDatabase database(db_path);
        SqliteStore store(database);
        Heaven heaven(store);
        Earth earth(heaven);

        const DeviceToken token_created = *DeviceToken::parse(token_text);
        assert(earth.soul_id_for_token(token_created) == *created_id);
        assert(heaven.find_by_id(*created_id)->name() == *SoulName::parse("abcdefgh"));

        const std::optional<Soul> a = heaven.find_by_id(*soul_a_id);
        assert(a.has_value());
        assert(a->name() == *SoulName::parse("nameaaaa"));
        assert(earth.soul_id_for_token(*DeviceToken::parse("aaaa1234aaaa1234aaaa1234aaaa1234")) == *soul_a_id);

        const std::optional<Soul> b = heaven.find_by_id(*soul_b_id);
        assert(b.has_value());
        assert(b->name() == *SoulName::parse("namebbbb"));

        assert(!heaven.find_by_id(id::Soul{999999}).has_value());
        assert(!earth.find_man_by_token(*DeviceToken::parse("ffffffffffffffffffffffffffffffff")).has_value());
    }

    ::unlink(db_path.c_str());
    return EXIT_SUCCESS;
}
