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

    assert(!registry.bind_user(session1, will::domain::UserId{7}));
    assert(registry.is_authenticated(session1));

    const auto displaced = registry.bind_user(session2, will::domain::UserId{7});
    assert(displaced && *displaced == session1);
    assert(!registry.is_authenticated(session1));
    assert(registry.is_authenticated(session2));
    assert(*registry.user_id(session2) == will::domain::UserId{7});
}


void test_same_session_rebind()
{
    will::SessionRegistry registry;
    const will::SessionId session = registry.register_test_session()->id();

    assert(!registry.bind_user(session, will::domain::UserId{7}));
    assert(!registry.bind_user(session, will::domain::UserId{7}));
    assert(registry.is_authenticated(session));
    assert(*registry.user_id(session) == will::domain::UserId{7});
}


void test_unregister_after_displace_keeps_new_owner()
{
    will::SessionRegistry registry;
    const will::SessionId session1 = registry.register_test_session()->id();
    const will::SessionId session2 = registry.register_test_session()->id();

    assert(!registry.bind_user(session1, will::domain::UserId{7}));
    const auto displaced = registry.bind_user(session2, will::domain::UserId{7});
    assert(displaced && *displaced == session1);

    registry.unregister_session(session1);
    assert(registry.is_authenticated(session2));
    assert(*registry.user_id(session2) == will::domain::UserId{7});

    registry.unregister_session(session2);
    assert(!registry.is_authenticated(session2));
}


void test_rebind_different_user_clears_old_index()
{
    will::SessionRegistry registry;
    const will::SessionId session1 = registry.register_test_session()->id();
    const will::SessionId session2 = registry.register_test_session()->id();

    assert(!registry.bind_user(session1, will::domain::UserId{7}));
    assert(!registry.bind_user(session1, will::domain::UserId{8}));

    assert(!registry.bind_user(session2, will::domain::UserId{7}));
    assert(registry.is_authenticated(session1));
    assert(registry.is_authenticated(session2));
    assert(*registry.user_id(session1) == will::domain::UserId{8});
    assert(*registry.user_id(session2) == will::domain::UserId{7});
}


} // namespace


int main()
{
    test_displace_other_session();
    test_same_session_rebind();
    test_unregister_after_displace_keeps_new_owner();
    test_rebind_different_user_clears_old_index();

    std::cout << "session registry test passed\n";
    return EXIT_SUCCESS;
}
