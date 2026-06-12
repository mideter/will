#include "sqlite_auth_session_store_impl.h"
#include "sqlite_database.h"
#include "sqlite_message_repository_impl.h"
#include "sqlite_otp_store_impl.h"
#include "sqlite_user_repository_impl.h"

#include "entities/chat_id.h"
#include "entities/otp_challenge.h"
#include "entities/user_id.h"

#include <cassert>
#include <cstdlib>
#include <string>
#include <unistd.h>


int main()
{
    using namespace will;
    using namespace will::domain;

    const std::string db_path = "/tmp/will-sqlite-persistence-test-" + std::to_string(getpid()) + ".db";
    ::unlink(db_path.c_str());

    SqliteDatabase database(db_path);
    SqliteMessageRepositoryImpl messages(database);
    SqliteUserRepositoryImpl users(database);
    SqliteAuthSessionStoreImpl sessions(database);
    SqliteOtpStoreImpl otp_store(database);

    const UserId peer_a{42};
    const UserId peer_b{99};
    const ChatId chat = ChatId::global();

    messages.append(chat, peer_a, "from-peer", 1000);
    messages.append(chat, peer_b, "from-me", 2000);

    const auto rows = messages.load_last(chat, 10);
    assert(rows.size() == 2);
    assert(rows[0].body == "from-peer");
    assert(rows[0].author_id == peer_a);
    assert(rows[1].body == "from-me");
    assert(rows[1].author_id == peer_b);

    const User created = users.create_user("+15551234567");
    assert(created.id.value > 0);
    assert(created.phone == "+15551234567");

    const std::optional<User> found = users.find_by_phone("+15551234567");
    assert(found.has_value());
    assert(found->id == created.id);

    const AuthToken token = sessions.issue_session(created.id);
    assert(!token.empty());

    const std::optional<Account> resolved = sessions.resolve_token(token);
    assert(resolved.has_value());
    assert(resolved->user_id == created.id);

    OtpChallenge challenge;
    challenge.phone = "+15559876543";
    challenge.code_hash = "hash-abc";
    challenge.expires_at_ms = 9'000;
    challenge.attempts = 0;
    challenge.peer_ip = "10.0.0.1";
    otp_store.save_challenge(challenge);

    const std::optional<OtpChallenge> loaded = otp_store.find_challenge("+15559876543");
    assert(loaded.has_value());
    assert(loaded->phone == challenge.phone);
    assert(loaded->code_hash == challenge.code_hash);
    assert(loaded->expires_at_ms == challenge.expires_at_ms);
    assert(loaded->attempts == 0);
    assert(loaded->peer_ip == challenge.peer_ip);

    otp_store.increment_attempts("+15559876543");
    const std::optional<OtpChallenge> after_attempt = otp_store.find_challenge("+15559876543");
    assert(after_attempt.has_value());
    assert(after_attempt->attempts == 1);

    OtpChallenge other_ip;
    other_ip.phone = "+15551112222";
    other_ip.code_hash = "hash-def";
    other_ip.expires_at_ms = 8'000;
    other_ip.attempts = 0;
    other_ip.peer_ip = "10.0.0.1";
    otp_store.save_challenge(other_ip);

    assert(otp_store.count_challenges_by_peer_ip("10.0.0.1", 7'500) == 2);
    assert(otp_store.count_challenges_by_peer_ip("10.0.0.1", 8'500) == 1);
    assert(otp_store.count_challenges_by_peer_ip("192.168.1.1", 0) == 0);

    challenge.code_hash = "hash-updated";
    challenge.expires_at_ms = 12'000;
    challenge.attempts = 2;
    otp_store.save_challenge(challenge);
    const std::optional<OtpChallenge> upserted = otp_store.find_challenge("+15559876543");
    assert(upserted.has_value());
    assert(upserted->code_hash == "hash-updated");
    assert(upserted->expires_at_ms == 12'000);
    assert(upserted->attempts == 2);

    otp_store.invalidate("+15559876543");
    assert(!otp_store.find_challenge("+15559876543").has_value());
    assert(otp_store.find_challenge("+15551112222").has_value());

    ::unlink(db_path.c_str());
    return EXIT_SUCCESS;
}
