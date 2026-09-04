#include "entities/world.h"
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
#include <optional>
#include <string>
#include <unistd.h>


int main()
{
	using namespace will;
	using namespace will::domain;

	const std::string db_path = "/tmp/will-sqlite-persistence-test-" + std::to_string(getpid()) + ".db";
	::unlink(db_path.c_str());

	std::optional<id::Soul> soul_a_id;
	std::optional<id::Soul> soul_b_id;
	std::optional<id::Soul> created_id;
	std::optional<SoulName> name_a;
	std::optional<SoulName> name_b;
	std::optional<SoulName> name_created;
	const std::string token_text = "abcd1234abcd1234abcd1234abcd1234";

	{
		SqliteDatabase database(db_path);
		SqliteStore store(database);
		SqliteLetterRepositoryImpl letters(database);
		World world(store);

		const DeviceToken token_a = *DeviceToken::parse("aaaa1234aaaa1234aaaa1234aaaa1234");
		const DeviceToken token_b = *DeviceToken::parse("bbbb1234bbbb1234bbbb1234bbbb1234");
		const DeviceToken token_created = *DeviceToken::parse(token_text);

		const Man man_a = world.birth_man(token_a);
		const Man man_b = world.birth_man(token_b);
		const Soul soul_a = *world.find_by_id(man_a.soul_id());
		const Soul soul_b = *world.find_by_id(man_b.soul_id());
		soul_a_id = soul_a.id();
		soul_b_id = soul_b.id();
		name_a = soul_a.name();
		name_b = soul_b.name();

		const id::Abode abode = world.abode().id;
		letters.append(abode, soul_a.id(), "from-peer", Timestamp{1000});
		letters.append(abode, soul_b.id(), "from-me", Timestamp{2000});

		const auto rows = letters.load_last(abode, 10);
		assert(rows.size() == 2);
		assert(rows[0].body() == "from-peer");
		assert(rows[0].author_id() == soul_a.id());
		assert(world.find_by_id(rows[0].author_id())->name() == *name_a);
		assert(rows[1].body() == "from-me");
		assert(rows[1].author_id() == soul_b.id());
		assert(world.find_by_id(rows[1].author_id())->name() == *name_b);

		const Man man_created = world.birth_man(token_created);
		const Soul created = *world.find_by_id(man_created.soul_id());
		created_id = created.id();
		name_created = created.name();
		assert(created.id().value() > 0);
		assert(world.soul_id_for_token(token_created) == created.id());

		assert(world.find_man_by_token(token_created).has_value());
		assert(world.find_by_id(created.id()).has_value());
	}

	{
		SqliteDatabase database(db_path);
		SqliteStore store(database);
		World world(store);

		const DeviceToken token_created = *DeviceToken::parse(token_text);
		assert(world.soul_id_for_token(token_created) == *created_id);
		assert(world.find_by_id(*created_id)->name() == *name_created);

		const std::optional<Soul> a = world.find_by_id(*soul_a_id);
		assert(a.has_value());
		assert(a->name() == *name_a);
		assert(world.soul_id_for_token(*DeviceToken::parse("aaaa1234aaaa1234aaaa1234aaaa1234")) == *soul_a_id);

		const std::optional<Soul> b = world.find_by_id(*soul_b_id);
		assert(b.has_value());
		assert(b->name() == *name_b);

		assert(!world.find_by_id(id::Soul{999999}).has_value());
		assert(!world.find_man_by_token(*DeviceToken::parse("ffffffffffffffffffffffffffffffff")).has_value());
	}

	::unlink(db_path.c_str());
	return EXIT_SUCCESS;
}
