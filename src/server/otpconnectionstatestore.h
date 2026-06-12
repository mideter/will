#pragma once

#include <asio.hpp>

#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>


namespace will {


enum class OtpConnectionPhase { None, AwaitingCode };


class TcpConnectionRegistry;


/**
 * Per-connection OTP handshake phase and pending phone.
 * Auth-pending timers run on the connection strand.
 */
class OtpConnectionStateStore {
public:
    OtpConnectionStateStore(TcpConnectionRegistry& registry, int auth_pending_timeout_sec);

    [[nodiscard]] OtpConnectionPhase phase(std::uint64_t connection_id) const;
    [[nodiscard]] std::optional<std::string> pending_phone(std::uint64_t connection_id) const;

    void begin_awaiting_code(std::uint64_t connection_id, std::string phone,
                             std::function<void(std::uint64_t)> on_timeout);
    void clear(std::uint64_t connection_id);

private:
    struct Entry {
        OtpConnectionPhase phase = OtpConnectionPhase::None;
        std::string pending_phone;
        std::shared_ptr<asio::steady_timer> timer;
        std::uint64_t timer_generation = 0;
    };

    void cancel_timer(Entry& entry);

    TcpConnectionRegistry& registry_;
    int auth_pending_timeout_sec_;
    mutable std::mutex mutex_;
    std::unordered_map<std::uint64_t, Entry> entries_;
};


} // namespace will
