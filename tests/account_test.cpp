#include "entities/account.h"
#include "entities/device_token.h"

#include <cassert>
#include <cstdlib>
#include <stdexcept>


int main()
{
    using namespace will::domain;

    const DeviceToken token = *DeviceToken::parse("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

    {
        const Account account{UserId{7}, token, Timestamp{500}};
        assert(account.user_id() == UserId{7});
        assert(account.device_token() == token);
        assert(account.authenticated_at() == Timestamp{500});
    }

    try {
        Account{UserId{0}, token, Timestamp{0}};
        return EXIT_FAILURE;
    } catch (const std::invalid_argument&) {
    }

    return EXIT_SUCCESS;
}
