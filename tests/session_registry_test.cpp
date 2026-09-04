#include "sessionregistry.h"

#include <cassert>
#include <cstdlib>
#include <iostream>


namespace {


void test_displace_other_session()
{
	will::SessionRegistry registry;
	const will::SessionId session1 = registry.register_test_session()->id();
	const will::SessionId session2 = registry.register_test_session()->id();

	assert(!registry.bind_soul(session1, will::domain::id::Soul{7}));
	assert(registry.is_authenticated(session1));

	const auto displaced = registry.bind_soul(session2, will::domain::id::Soul{7});
	assert(displaced && *displaced == session1);
	assert(!registry.is_authenticated(session1));
	assert(registry.is_authenticated(session2));
	assert(*registry.soul_id(session2) == will::domain::id::Soul{7});
}


void test_same_session_rebind()
{
	will::SessionRegistry registry;
	const will::SessionId session = registry.register_test_session()->id();

	assert(!registry.bind_soul(session, will::domain::id::Soul{7}));
	assert(!registry.bind_soul(session, will::domain::id::Soul{7}));
	assert(registry.is_authenticated(session));
	assert(*registry.soul_id(session) == will::domain::id::Soul{7});
}


void test_unregister_after_displace_keeps_new_owner()
{
	will::SessionRegistry registry;
	const will::SessionId session1 = registry.register_test_session()->id();
	const will::SessionId session2 = registry.register_test_session()->id();

	assert(!registry.bind_soul(session1, will::domain::id::Soul{7}));
	const auto displaced = registry.bind_soul(session2, will::domain::id::Soul{7});
	assert(displaced && *displaced == session1);

	registry.unregister_session(session1);
	assert(registry.is_authenticated(session2));
	assert(*registry.soul_id(session2) == will::domain::id::Soul{7});

	registry.unregister_session(session2);
	assert(!registry.is_authenticated(session2));
}


void test_rebind_different_soul_clears_old_index()
{
	will::SessionRegistry registry;
	const will::SessionId session1 = registry.register_test_session()->id();
	const will::SessionId session2 = registry.register_test_session()->id();

	assert(!registry.bind_soul(session1, will::domain::id::Soul{7}));
	assert(!registry.bind_soul(session1, will::domain::id::Soul{8}));

	assert(!registry.bind_soul(session2, will::domain::id::Soul{7}));
	assert(registry.is_authenticated(session1));
	assert(registry.is_authenticated(session2));
	assert(*registry.soul_id(session1) == will::domain::id::Soul{8});
	assert(*registry.soul_id(session2) == will::domain::id::Soul{7});
}


} // namespace


int main()
{
	test_displace_other_session();
	test_same_session_rebind();
	test_unregister_after_displace_keeps_new_owner();
	test_rebind_different_soul_clears_old_index();

	std::cout << "session registry test passed\n";
	return EXIT_SUCCESS;
}
