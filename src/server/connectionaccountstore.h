#pragma once

#include "entities/account.h"

#include <cstdint>
#include <mutex>
#include <optional>
#include <unordered_map>


namespace will {


/**
 * Thread-safe store of authenticated session context keyed by connection id.
 */
class ConnectionAccountStore {
public:
    [[nodiscard]] bool has(std::uint64_t connection_id) const;
    [[nodiscard]] std::optional<domain::Account> get(std::uint64_t connection_id) const;
    void set(std::uint64_t connection_id, domain::Account account);
    void remove(std::uint64_t connection_id);

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::uint64_t, domain::Account> accounts_;
};


} // namespace will
