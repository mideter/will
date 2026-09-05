#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "sessionregistry.h"


TEST_CASE("displace other session")
{
	will::SessionRegistry registry;
	const will::SessionId session1 = registry.register_test_session()->id();
	const will::SessionId session2 = registry.register_test_session()->id();

	CHECK_FALSE(registry.bind_soul(session1, will::domain::id::Soul{7}));
	CHECK(registry.is_authenticated(session1));

	const auto displaced = registry.bind_soul(session2, will::domain::id::Soul{7});
	REQUIRE(displaced);
	CHECK(*displaced == session1);
	CHECK_FALSE(registry.is_authenticated(session1));
	CHECK(registry.is_authenticated(session2));
	CHECK(*registry.soul_id(session2) == will::domain::id::Soul{7});
}


TEST_CASE("same session rebind")
{
	will::SessionRegistry registry;
	const will::SessionId session = registry.register_test_session()->id();

	CHECK_FALSE(registry.bind_soul(session, will::domain::id::Soul{7}));
	CHECK_FALSE(registry.bind_soul(session, will::domain::id::Soul{7}));
	CHECK(registry.is_authenticated(session));
	CHECK(*registry.soul_id(session) == will::domain::id::Soul{7});
}


TEST_CASE("unregister after displace keeps new owner")
{
	will::SessionRegistry registry;
	const will::SessionId session1 = registry.register_test_session()->id();
	const will::SessionId session2 = registry.register_test_session()->id();

	CHECK_FALSE(registry.bind_soul(session1, will::domain::id::Soul{7}));
	const auto displaced = registry.bind_soul(session2, will::domain::id::Soul{7});
	REQUIRE(displaced);
	CHECK(*displaced == session1);

	registry.unregister_session(session1);
	CHECK(registry.is_authenticated(session2));
	CHECK(*registry.soul_id(session2) == will::domain::id::Soul{7});

	registry.unregister_session(session2);
	CHECK_FALSE(registry.is_authenticated(session2));
}


TEST_CASE("rebind different soul clears old index")
{
	will::SessionRegistry registry;
	const will::SessionId session1 = registry.register_test_session()->id();
	const will::SessionId session2 = registry.register_test_session()->id();

	CHECK_FALSE(registry.bind_soul(session1, will::domain::id::Soul{7}));
	CHECK_FALSE(registry.bind_soul(session1, will::domain::id::Soul{8}));

	CHECK_FALSE(registry.bind_soul(session2, will::domain::id::Soul{7}));
	CHECK(registry.is_authenticated(session1));
	CHECK(registry.is_authenticated(session2));
	CHECK(*registry.soul_id(session1) == will::domain::id::Soul{8});
	CHECK(*registry.soul_id(session2) == will::domain::id::Soul{7});
}
