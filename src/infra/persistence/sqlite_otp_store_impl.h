#pragma once

#include "ports/otp_store.h"
#include "sqlite_database.h"

#include <string_view>


namespace will {


class SqliteOtpStoreImpl final : public domain::OtpStore {
public:
    explicit SqliteOtpStoreImpl(SqliteDatabase& database);

    void save_challenge(domain::OtpChallenge challenge) override;
    std::optional<domain::OtpChallenge> find_challenge(std::string_view phone) override;
    void increment_attempts(std::string_view phone) override;
    void invalidate(std::string_view phone) override;
    std::size_t count_challenges_by_peer_ip(std::string_view peer_ip,
                                            domain::TimestampMs since_ms) override;

private:
    SqliteDatabase& database_;
};


} // namespace will
