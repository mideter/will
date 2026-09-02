#include "domain_fakes.h"

#include "entities/heaven.h"
#include "ids/abode_id.h"
#include "values/device_token.h"
#include "values/timestamp.h"
#include "values/god_name.h"
#include "errors/auth_error.h"
#include "errors/domain_error.h"
#include "usecases/authenticate_device.h"
#include "usecases/fetch_letter_history.h"
#include "usecases/send_letter.h"

#include <cassert>
#include <cstdlib>
#include <variant>


namespace {


using namespace will::domain;
using namespace will::domain::test;


DeviceToken test_token(const char* hex = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
{
    return *DeviceToken::parse(hex);
}


GodName test_name(const char* text)
{
    return *GodName::parse(text);
}


void test_authenticate_device_creates_god()
{
    InMemoryEternity eternity;
    Heaven heaven(eternity);
    AuthenticateDevice authenticate(heaven);

    const DeviceToken token = DeviceToken::generate();
    const auto result = authenticate.execute(AuthenticateDeviceInput{token.text()});
    assert(std::holds_alternative<AuthenticateDeviceSuccess>(result));

    const AuthenticateDeviceSuccess& success = std::get<AuthenticateDeviceSuccess>(result);
    assert(success.god.id().value() > 0);

    const std::optional<God> god = heaven.find_by_device_token(token.text());
    assert(god.has_value());
    assert(god->id() == success.god.id());
    assert(GodName::parse(god->name().text()));
}


void test_authenticate_device_existing_god()
{
    InMemoryEternity eternity;
    Heaven heaven(eternity);
    seed_god_with_token(heaven, GodId{42}, test_token("abcd1234abcd1234abcd1234abcd1234"), test_name("oldname1"));

    AuthenticateDevice authenticate(heaven);
    const auto result = authenticate.execute(AuthenticateDeviceInput{"abcd1234abcd1234abcd1234abcd1234"});
    assert(std::holds_alternative<AuthenticateDeviceSuccess>(result));
    assert(std::get<AuthenticateDeviceSuccess>(result).god.id() == GodId{42});
}


void test_authenticate_device_keeps_existing_name()
{
    InMemoryEternity eternity;
    Heaven heaven(eternity);
    seed_god_with_token(heaven, GodId{7}, test_token("abcd1234abcd1234abcd1234abcd1234"), test_name("keptname"));

    AuthenticateDevice authenticate(heaven);
    const auto result = authenticate.execute(AuthenticateDeviceInput{"abcd1234abcd1234abcd1234abcd1234"});
    assert(std::holds_alternative<AuthenticateDeviceSuccess>(result));

    const std::optional<God> god = heaven.find_by_device_token("abcd1234abcd1234abcd1234abcd1234");
    assert(god.has_value());
    assert(god->name() == test_name("keptname"));
}


void test_authenticate_device_invalid_token()
{
    InMemoryEternity eternity;
    Heaven heaven(eternity);
    AuthenticateDevice authenticate(heaven);

    const auto result = authenticate.execute(AuthenticateDeviceInput{"short"});
    assert(std::holds_alternative<AuthError>(result));
    assert(std::get<AuthError>(result) == AuthError::InvalidToken);
}


void test_send_letter_persists_and_notifies()
{
    InMemoryLetterRepository letters;
    FakeParticipantNotifier notifier;

    const GodId author{7};

    SendLetter send(letters, notifier);
    const Letter saved = send.execute(SendLetterInput{author, AbodeId::global(), "hello", Timestamp{900}});

    assert(saved.id().value() > 0);
    assert(saved.author_id() == author);
    assert(saved.body() == "hello");
    assert(saved.created_at() == Timestamp{900});

    const auto loaded = letters.load_last(AbodeId::global(), 10);
    assert(loaded.size() == 1);
    assert(loaded[0].body() == "hello");

    assert(notifier.notifications_.size() == 1);
    assert(notifier.notifications_[0].id() == saved.id());
}


void test_fetch_letter_history_limit_and_is_mine()
{
    InMemoryLetterRepository letters;
    InMemoryEternity eternity;
    Heaven heaven(eternity);
    const AbodeId abode = AbodeId::global();
    const GodId me{10};
    const GodId other{20};

    seed_god_with_token(heaven, me, test_token("c0ffee00c0ffee00c0ffee00c0ffee00"), test_name("menameaa"));
    seed_god_with_token(heaven, other, test_token("deadbeefdeadbeefdeadbeefdeadbeef"), test_name("peername"));

    letters.append(abode, other, "peer", Timestamp{1});
    letters.append(abode, me, "mine", Timestamp{2});
    letters.append(abode, other, "peer2", Timestamp{3});

    FetchLetterHistory fetch(letters, heaven);

    const auto zero_limit = fetch.execute(FetchLetterHistoryInput{me, abode, 0});
    assert(std::holds_alternative<DomainError>(zero_limit));
    assert(std::get<DomainError>(zero_limit).code == DomainErrorCode::InvalidArgument);

    const auto ok = fetch.execute(FetchLetterHistoryInput{me, abode, 2});
    assert(std::holds_alternative<FetchLetterHistoryResult>(ok));

    const FetchLetterHistoryResult& result = std::get<FetchLetterHistoryResult>(ok);
    assert(result.items.size() == 2);
    assert(result.items[0].letter.body() == "mine");
    assert(result.items[0].author_name == "menameaa");
    assert(result.items[0].is_mine);
    assert(result.items[1].letter.body() == "peer2");
    assert(result.items[1].author_name == "peername");
    assert(!result.items[1].is_mine);
}


void test_fetch_letter_history_caps_limit()
{
    InMemoryLetterRepository letters;
    InMemoryEternity eternity;
    Heaven heaven(eternity);
    const AbodeId abode = AbodeId::global();
    const GodId author{1};

    seed_god_with_token(heaven, author, test_token("feedfacefeedfacefeedfacefeedface"), test_name("authoraa"));

    for (int i = 0; i < 5; ++i)
        letters.append(abode, author, "m", Timestamp{i});

    FetchLetterHistory fetch(letters, heaven);
    const auto ok = fetch.execute(FetchLetterHistoryInput{
        author, abode, FetchLetterHistory::MaxHistoryRequestLimit + 50});
    assert(std::holds_alternative<FetchLetterHistoryResult>(ok));
    assert(std::get<FetchLetterHistoryResult>(ok).items.size() == 5);
}


} // namespace


int main()
{
    test_authenticate_device_creates_god();
    test_authenticate_device_existing_god();
    test_authenticate_device_keeps_existing_name();
    test_authenticate_device_invalid_token();
    test_send_letter_persists_and_notifies();
    test_fetch_letter_history_limit_and_is_mine();
    test_fetch_letter_history_caps_limit();
    return EXIT_SUCCESS;
}
