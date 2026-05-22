#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>


namespace will {


class Session;


/**
 * Thread-safe registry of connected {@link Session} instances.
 */
class SessionRegistry {
public:
    SessionRegistry() = default;
    ~SessionRegistry() = default;

    SessionRegistry(const SessionRegistry&) = delete;
    SessionRegistry& operator=(const SessionRegistry&) = delete;
    SessionRegistry(SessionRegistry&&) = delete;
    SessionRegistry& operator=(SessionRegistry&&) = delete;

    void add(std::shared_ptr<Session> session);
    void remove(std::uint64_t session_id);

    /** Closes all sessions and clears the registry. */
    void shutdown_all();

    std::size_t count() const noexcept;

    /** Logs and fan-out of {@code payload} to every session except {@code sender}. */
    void broadcast_except(const Session& sender, const std::vector<char>& payload);

    bool at_capacity(std::size_t max_connections) const noexcept;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::uint64_t, std::shared_ptr<Session>> sessions_;
};


} // namespace will
