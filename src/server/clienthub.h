#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>


namespace will {


class Session;


/**
 * Thread-safe registry of connected {@link Session} instances.
 */
class ClientHub {
public:
    ClientHub() = default;
    ~ClientHub() = default;

    ClientHub(const ClientHub&) = delete;
    ClientHub& operator=(const ClientHub&) = delete;
    ClientHub(ClientHub&&) = delete;
    ClientHub& operator=(ClientHub&&) = delete;

    void add(std::shared_ptr<Session> session);
    void remove(std::uint64_t session_id);
    void reset();

    /** Closes all sessions and clears the registry. */
    void shutdown_all();

    std::size_t count() const noexcept;

    /** Invokes {@code fn} for each session except {@code except_id} (under hub lock). */
    void broadcast_except(std::uint64_t except_id, const std::vector<char>& payload,
                          const std::function<void(const std::shared_ptr<Session>&)>& enqueue_fn);

    bool at_capacity(std::size_t max_connections) const noexcept;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::uint64_t, std::shared_ptr<Session>> sessions_;
};


} // namespace will
