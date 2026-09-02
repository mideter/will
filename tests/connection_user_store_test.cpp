#include "connectionuserstore.h"

#include "entities/device_token.h"
#include "entities/user_name.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <optional>


namespace {


will::domain::DeviceToken test_token(const char* hex)
{
    return *will::domain::DeviceToken::parse(hex);
}


will::domain::User make_user(std::uint64_t user_id,
                             const char* token = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
{
    return will::domain::User{will::domain::UserId{user_id}, test_token(token),
                              *will::domain::UserName::parse("testuser")};
}


void test_displace_other_connection()
{
    will::ConnectionUserStore store;

    assert(!store.set(1, make_user(7)));
    assert(store.has(1));

    const auto displaced = store.set(2, make_user(7));
    assert(displaced && *displaced == 1);
    assert(!store.has(1));
    assert(store.has(2));
    assert(store.get(2)->id() == will::domain::UserId{7});
}


void test_same_connection_rebind()
{
    will::ConnectionUserStore store;

    assert(!store.set(1, make_user(7)));
    assert(!store.set(1, make_user(7, "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb")));
    assert(store.has(1));
    assert(store.get(1)->device_token().text() == "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
}


void test_remove_after_displace_keeps_new_owner()
{
    will::ConnectionUserStore store;

    assert(!store.set(1, make_user(7)));
    const auto displaced = store.set(2, make_user(7));
    assert(displaced && *displaced == 1);

    store.remove(1);
    assert(store.has(2));
    assert(store.get(2)->id() == will::domain::UserId{7});

    store.remove(2);
    assert(!store.has(2));
}


void test_rebind_different_user_clears_old_index()
{
    will::ConnectionUserStore store;

    assert(!store.set(1, make_user(7)));
    assert(!store.set(1, make_user(8)));

    assert(!store.set(2, make_user(7)));
    assert(store.has(1));
    assert(store.has(2));
    assert(store.get(1)->id() == will::domain::UserId{8});
    assert(store.get(2)->id() == will::domain::UserId{7});
}


} // namespace


int main()
{
    test_displace_other_connection();
    test_same_connection_rebind();
    test_remove_after_displace_keeps_new_owner();
    test_rebind_different_user_clears_old_index();

    std::cout << "connection user store test passed\n";
    return EXIT_SUCCESS;
}
