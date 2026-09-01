#include "connectionaccountstore.h"

#include "entities/device_token.h"
#include "entities/timestamp.h"
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


will::domain::Account make_account(std::uint64_t user_id,
                                   const char* token = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
{
    return will::domain::Account{will::domain::UserId{user_id}, test_token(token),
                                 will::domain::Timestamp{1000}, *will::domain::UserName::parse("testuser")};
}


void test_displace_other_connection()
{
    will::ConnectionAccountStore store;

    assert(!store.set(1, make_account(7)));
    assert(store.has(1));

    const auto displaced = store.set(2, make_account(7));
    assert(displaced && *displaced == 1);
    assert(!store.has(1));
    assert(store.has(2));
    assert(store.get(2)->user_id == will::domain::UserId{7});
}


void test_same_connection_rebind()
{
    will::ConnectionAccountStore store;

    assert(!store.set(1, make_account(7)));
    assert(!store.set(1, make_account(7, "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb")));
    assert(store.has(1));
    assert(store.get(1)->device_token.text() == "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
}


void test_remove_after_displace_keeps_new_owner()
{
    will::ConnectionAccountStore store;

    assert(!store.set(1, make_account(7)));
    const auto displaced = store.set(2, make_account(7));
    assert(displaced && *displaced == 1);

    store.remove(1);
    assert(store.has(2));
    assert(store.get(2)->user_id == will::domain::UserId{7});

    store.remove(2);
    assert(!store.has(2));
}


void test_rebind_different_user_clears_old_index()
{
    will::ConnectionAccountStore store;

    assert(!store.set(1, make_account(7)));
    assert(!store.set(1, make_account(8)));

    assert(!store.set(2, make_account(7)));
    assert(store.has(1));
    assert(store.has(2));
    assert(store.get(1)->user_id == will::domain::UserId{8});
    assert(store.get(2)->user_id == will::domain::UserId{7});
}


} // namespace


int main()
{
    test_displace_other_connection();
    test_same_connection_rebind();
    test_remove_after_displace_keeps_new_owner();
    test_rebind_different_user_clears_old_index();

    std::cout << "connection account store test passed\n";
    return EXIT_SUCCESS;
}
